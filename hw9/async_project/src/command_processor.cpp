/**
 * @file command_processor.cpp
 * @brief Реализация классов для многопоточной обработки команд
 * 
 * Содержит реализацию ThreadPool и CommandProcessor для асинхронной
 * обработки пакетов команд с выводом в консоль и файлы.
 */

#include "command_processor.h"
#include <sstream>

/**
 * @brief Конструктор пула потоков
 * @param num_threads Количество рабочих потоков в пуле
 * @param name Имя пула для отладки и идентификации
 * 
 * Создает указанное количество рабочих потоков, которые начинают
 * ожидать задачи в очереди. Каждый поток работает в бесконечном цикле,
 * извлекая и выполняя задачи из очереди.
 * 
 * @note Потоки создаются сразу при построении объекта
 * @note Имя пула используется только для отладочного вывода
 */
ThreadPool::ThreadPool(size_t num_threads, const std::string& name) 
    : pool_name_(name) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this, i, name] {
            // Отладочный вывод при запуске потока
            //std::cout << "Thread " << name << "-" << i << " started\n";
            
            // Основной цикл выполнения задач
            while (true) {
                std::function<void()> task;
                {
                    // Ожидаем появления задачи или сигнала остановки
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    condition_.wait(lock, [this] {
                        return stop_.load() || !tasks_.empty();
                    });
                    
                    // Проверяем условие выхода из цикла
                    if (stop_.load() && tasks_.empty()) return;
                    if (tasks_.empty()) continue;
                    
                    // Извлекаем задачу из очереди
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                
                // Выполняем задачу вне критической секции
                task();
            }
        });
    }
}

/**
 * @brief Принудительная остановка всех потоков пула
 * 
 * Устанавливает флаг остановки, уведомляет все ожидающие потоки
 * и дожидается их завершения через join(). Гарантирует, что все
 * потоки будут корректно остановлены перед возвратом управления.
 * 
 * @note После вызова shutdown() добавление новых задач невозможно
 * @note Метод является идемпотентным - повторные вызовы безопасны
 */
void ThreadPool::shutdown() {
    stop_.store(true);
    condition_.notify_all();
    for (std::thread &worker : workers_) {
        if (worker.joinable()) {
            worker.join();  // Ожидаем завершения потока
        }
    }
}

/**
 * @brief Деструктор пула потоков
 * 
 * Автоматически вызывает shutdown() для корректного завершения
 * всех рабочих потоков. Гарантирует освобождение всех ресурсов.
 * 
 * @warning Не должен вызываться при наличии активных ссылок на объект
 */
ThreadPool::~ThreadPool() {
    stop_.store(true);
    condition_.notify_all();
    for (std::thread &worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

// CommandProcessor implementation

/**
 * @brief Конструктор обработчика команд
 * @param bulk_size Размер блока команд для автоматической обработки
 * 
 * Инициализирует обработчик команд и создает три фоновых потока:
 * - 1 поток для вывода в консоль (log)
 * - 2 потока для записи в файлы (file1, file2)
 * 
 * @note Потоки запускаются сразу после создания
 */
CommandProcessor::CommandProcessor(size_t bulk_size) 
    : bulk_size_(bulk_size) {
    // Создаем ТРИ потока:
    log_thread_pool_ = std::make_unique<ThreadPool>(1, "log");    // 1 поток для консоли
    file_thread_pool_ = std::make_unique<ThreadPool>(2, "file");  // 2 потока для файлов
}

/**
 * @brief Деструктор обработчика команд
 * 
 * Автоматически обрабатывает оставшиеся команды и останавливает
 * все фоновые потоки. Гарантирует корректное освобождение ресурсов.
 * 
 * @note Вызывает flush_remaining() для обработки неполных блоков
 */
CommandProcessor::~CommandProcessor() {
    flush_remaining();
}

/**
 * @brief Принудительная остановка всех фоновых потоков
 * 
 * Последовательно останавливает пулы потоков для консоли и файлов.
 * После вызова обработчик не может использоваться для обработки команд.
 * 
 * @note Безопасен для повторного вызова
 * @note Останавливает потоки в порядке: log, затем file
 */
void CommandProcessor::shutdown() {
    if (log_thread_pool_) {
        log_thread_pool_->shutdown();
    }
    if (file_thread_pool_) {
        file_thread_pool_->shutdown();
    }
}

/**
 * @brief Обрабатывает очередную команду
 * @param command Команда для обработки в виде string_view
 * 
 * Обрабатывает команды согласно следующим правилам:
 * - "{" - начинает динамический блок (увеличивает глубину вложенности)
 * - "}" - завершает динамический блок (уменьшает глубину вложенности)
 * - Обычные команды добавляются в текущий блок
 * 
 * Автоматически завершает блок при:
 * - Достижении размера bulk_size (вне динамических блоков)
 * - Завершении динамического блока (глубина вложенности становится 0)
 * - Начале нового динамического блока
 * 
 * @note Фиксирует время первой команды в блоке для генерации имени файла
 * @note Игнорирует лишние закрывающие скобки (глубина не становится отрицательной)
 */
void CommandProcessor::process_command(std::string_view command) {
    if (command == "{") {
        // Начало динамического блока
        if (block_depth_ == 0) {
            flush_commands(); // Завершаем предыдущий блок перед началом нового
        }
        block_depth_++;
    } else if (command == "}") {
        // Конец динамического блока
        if (block_depth_ > 0) {
            block_depth_--;
            if (block_depth_ == 0) {
                flush_commands(); // Завершаем блок при закрытии внешних скобок
            }
        }
    } else {
        // Обычная команда - добавляем в текущий блок
        if (commands_.empty()) {
            // Фиксируем время первой команды в блоке для имени файла
            first_command_time_ = std::chrono::system_clock::now();
        }
        commands_.emplace_back(command);
        
        // Автоматическое завершение блока при достижении размера (вне динамических блоков)
        if (block_depth_ == 0 && commands_.size() >= bulk_size_) {
            flush_commands();
        }
    }
}

/**
 * @brief Принудительная обработка оставшихся команд в буфере
 * 
 * Обрабатывает накопленные команды если обработчик не находится
 * внутри динамического блока. В противном случае очищает буфер
 * без обработки (игнорирует незавершенный блок).
 * 
 * @note Используется при завершении работы для обработки последних команд
 * @note Игнорирует команды в незавершенных динамических блоках
 */
void CommandProcessor::flush_remaining() {
    if (block_depth_ == 0) {
        flush_commands(); // Обрабатываем завершенный блок
    } else {
        commands_.clear(); // Игнорируем незавершенный динамический блок
    }
}

/**
 * @brief Отправляет текущий блок команд на обработку в фоновые потоки
 * 
 * Выполняет следующие действия:
 * 1. Создает копию блока команд для передачи в потоки
 * 2. Отправляет блок в поток вывода в консоль (log)
 * 3. Отправляет блок в один из файловых потоков (round-robin)
 * 4. Очищает буфер команд для приема новых данных
 * 
 * @note Генерирует уникальные имена файлов на основе временных меток
 * @note Использует move-семантику для эффективной передачи данных
 * @note Распределяет нагрузку между файловыми потоками по алгоритму round-robin
 */
void CommandProcessor::flush_commands() {
    if (commands_.empty()) return;
    
    // Сохраняем копию данных для передачи в потоки (потокобезопасность)
    auto commands_copy = commands_;
    auto timestamp = first_command_time_;
    
    // ВЫВОД В КОНСОЛЬ через отдельный поток (log)
    log_thread_pool_->enqueue_task([commands_copy]() {
        std::cout << "bulk: ";
        for (size_t i = 0; i < commands_copy.size(); ++i) {
            if (i != 0) std::cout << ", ";
            std::cout << commands_copy[i];
        }
        std::cout << std::endl;
    });
    
    // ЗАПИСЬ В ФАЙЛ через file1/file2 потоки (round-robin распределение)
    size_t file_index = file_writer_index_++ % 2;
    
    file_thread_pool_->enqueue_task([commands_copy = std::move(commands_copy), 
                                   timestamp, file_index]() {
        // Генерируем уникальное имя файла с высокой точностью
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        auto epoch = now_ms.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
        
        // Формат имени: bulk_<секунды>_<микросекунды>_<индекс_потока>.log
        std::string filename = "bulk_" + 
                              std::to_string(
                                  std::chrono::duration_cast<std::chrono::seconds>(
                                      timestamp.time_since_epoch()).count()) +
                              "_" + std::to_string(value.count()) +
                              "_" + std::to_string(file_index) + ".log";
        
        // Записываем блок команд в файл
        if (std::ofstream file{filename}) {
            file << "bulk: ";
            for (size_t i = 0; i < commands_copy.size(); ++i) {
                if (i != 0) file << ", ";
                file << commands_copy[i];
            }
            file << '\n'; // Используем '\n' вместо endl для эффективности
        }
    });
    
    // Очищаем буфер для приема новых команд
    commands_.clear();
}
