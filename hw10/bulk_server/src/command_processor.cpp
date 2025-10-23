#include "command_processor.h"
#include <algorithm>

/**
 * @brief Конструктор обработчика команд
 * @param bulk_size Размер блока команд для автоматической обработки
 * 
 * @details Инициализирует обработчик с заданным размером блока и создает
 * пул потоков для асинхронного уведомления наблюдателей. Пул потоков
 * используется для неблокирующей отправки уведомлений подписчикам.
 * 
 * @note Создает пул из 2 потоков с именем "notify" для обработки уведомлений
 */
CommandProcessor::CommandProcessor(size_t bulk_size) 
    : bulk_size_(bulk_size) {
    // Создаем пул потоков для асинхронных уведомлений
    notification_pool_ = std::make_unique<ThreadPool>(2, "notify");
}

/**
 * @brief Деструктор обработчика команд
 * 
 * @details Автоматически обрабатывает оставшиеся команды через flush_remaining()
 * и освобождает ресурсы пула потоков. Гарантирует корректное завершение работы
 * даже при исключительных ситуациях.
 * 
 * @note Вызывает flush_remaining() для обработки неполных блоков команд
 */
CommandProcessor::~CommandProcessor() {
    flush_remaining();
}

/**
 * @brief Подписывает наблюдателя на уведомления
 * @param observer Указатель на наблюдателя для подписки
 * 
 * @details Добавляет наблюдателя в список подписчиков. После подписки
 * наблюдатель будет получать уведомления о всех сформированных блоках команд.
 * Операция защищена мьютексом для обеспечения потокобезопасности.
 * 
 * @note Один и тот же наблюдатель может быть добавлен только один раз
 * @note Подписка происходит в конец списка наблюдателей
 */
void CommandProcessor::subscribe(std::shared_ptr<IObserver> observer) {
    std::lock_guard<std::mutex> lock(observers_mutex_);
    observers_.push_back(observer);
}

/**
 * @brief Отписывает наблюдателя от уведомлений
 * @param observer Указатель на наблюдателя для отписки
 * 
 * @details Удаляет наблюдателя из списка подписчиков используя линейный поиск.
 * Если наблюдатель не найден в списке, метод завершается без действий.
 * Операция защищена мьютексом для обеспечения потокобезопасности.
 * 
 * @note Время выполнения операции O(n), где n - количество подписчиков
 * @note Не выбрасывает исключений если наблюдатель не найден
 */
void CommandProcessor::unsubscribe(std::shared_ptr<IObserver> observer) {
    std::lock_guard<std::mutex> lock(observers_mutex_);
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

/**
 * @brief Уведомляет всех подписанных наблюдателей о новом блоке команд
 * @param commands Вектор команд в блоке
 * @param timestamp Временная метка создания блока
 * @param context_id Идентификатор контекста (соединения)
 * 
 * @details Создает копию списка наблюдателей для минимизации времени
 * блокировки мьютекса, затем асинхронно отправляет уведомления всем
 * наблюдателям через пул потоков. Каждое уведомление выполняется в
 * отдельной задаче пула потоков.
 * 
 * @note Не блокирует вызывающий поток на время обработки наблюдателями
 * @note Использует захват по значению для обеспечения безопасности данных
 */
void CommandProcessor::notify(const std::vector<std::string>& commands, 
                             time_t timestamp, 
                             void* context_id) {
    std::vector<std::shared_ptr<IObserver>> observers_copy;
    {
        std::lock_guard<std::mutex> lock(observers_mutex_);
        observers_copy = observers_;
    }
    
    // Асинхронно уведомляем всех наблюдателей
    for (auto& observer : observers_copy) {
        notification_pool_->enqueue_task([observer, commands, timestamp, context_id]() {
            observer->update(commands, timestamp, context_id);
        });
    }
}

/**
 * @brief Обрабатывает очередную команду
 * @param command Команда для обработки в виде string_view
 * 
 * @details Анализирует тип команды и выполняет соответствующие действия:
 * - "{" - начинает динамический блок, при необходимости завершая предыдущий
 * - "}" - завершает динамический блок, обрабатывая команды если глубина = 0
 * - Обычная команда - добавляется в буфер, проверяет условия завершения блока
 * 
 * @note Фиксирует время первой команды в блоке для генерации временной метки
 * @note Автоматически завершает блок при достижении bulk_size вне динамических блоков
 * @note Поддерживает вложенность динамических блоков через счетчик block_depth_
 */
void CommandProcessor::process_command(std::string_view command) {
    std::lock_guard<std::mutex> lock(command_mutex_);
    
    if (command == "{") {
        if (block_depth_ == 0) {
            flush_commands();
        }
        block_depth_++;
    } else if (command == "}") {
        if (block_depth_ > 0) {
            block_depth_--;
            if (block_depth_ == 0) {
                flush_commands();
            }
        }
    } else {
        if (commands_.empty()) {
            first_command_time_ = std::chrono::system_clock::now();
        }
        commands_.emplace_back(command);
        
        if (block_depth_ == 0 && commands_.size() >= bulk_size_) {
            flush_commands();
        }
    }
}

/**
 * @brief Принудительная обработка оставшихся команд в буфере
 * 
 * @details Проверяет текущее состояние обработчика: если не находится внутри
 * динамического блока (block_depth_ == 0), обрабатывает накопленные команды,
 * в противном случае очищает буфер без уведомления наблюдателей.
 * 
 * @note Используется при завершении работы для обработки последних команд
 * @note Игнорирует команды в незавершенных динамических блоках согласно ТЗ
 */
void CommandProcessor::flush_remaining() {
    std::lock_guard<std::mutex> lock(command_mutex_);
    if (block_depth_ == 0) {
        flush_commands();
    } else {
        commands_.clear();
    }
}

/**
 * @brief Отправляет текущий блок команд на обработку наблюдателям
 * 
 * @details Создает копии данных для передачи в фоновые потоки, преобразует
 * временную метку в формат time_t и вызывает метод notify() для уведомления
 * всех зарегистрированных наблюдателей. После успешной отправки очищает
 * буфер команд для приема новых данных.
 * 
 * @note Вызывается только когда буфер команд не пуст
 * @note Использует копирование данных для обеспечения потокобезопасности
 */
void CommandProcessor::flush_commands() {
    if (commands_.empty()) return;
    
    auto commands_copy = commands_;
    auto timestamp = std::chrono::system_clock::to_time_t(first_command_time_);
    
    // Уведомляем наблюдателей о новом блоке команд
    notify(commands_copy, timestamp, this);
    
    commands_.clear();
}

/**
 * @brief Принудительная остановка всех фоновых потоков
 * 
 * @details Вызывает shutdown() пула потоков для уведомлений, что приводит
 * к корректному завершению всех рабочих потоков. После вызова этого метода
 * асинхронные уведомления наблюдателей прекращаются.
 * 
 * @note Не влияет на уже отправленные уведомления - они будут обработаны
 * @note Безопасен для повторного вызова
 */
void CommandProcessor::shutdown() {
    if (notification_pool_) {
        notification_pool_->shutdown();
    }
}
