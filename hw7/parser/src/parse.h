#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

/**
 * @brief Базовый класс наблюдателя в паттерне "Наблюдатель"
 * 
 * Определяет интерфейс для объектов, которые должны быть уведомлены об изменениях
 */
class IObserver {
public:
    virtual ~IObserver() = default;
    
    /**
     * @brief Метод обновления состояния наблюдателя
     * @param commands Вектор команд для обработки
     * @param timestamp Временная метка первой команды в блоке
     */
    virtual void update(const std::vector<std::string>& commands, time_t timestamp) = 0;
};

/**
 * @brief Конкретный наблюдатель для вывода команд в консоль
 * 
 * Реализует вывод блока команд в стандартный поток вывода
 */
class ConsoleObserver : public IObserver {
public:
    /**
     * @brief Выводит блок команд в консоль
     * @param commands Вектор команд для вывода
     * @param timestamp Временная метка (не используется в этом наблюдателе)
     */
    void update(const std::vector<std::string>& commands, time_t timestamp) override {
        (void)timestamp; // Явно указываем, что параметр не используется
        if (commands.empty()) return;
        
        std::cout << "bulk: ";
        for (size_t i = 0; i < commands.size(); ++i) {
            if (i != 0) std::cout << ", ";
            std::cout << commands[i];
        }
        std::cout << std::endl;
    }
};

/**
 * @brief Конкретный наблюдатель для записи команд в файл
 * 
 * Реализует запись блока команд в файл с именем, содержащим временную метку
 */
class FileObserver : public IObserver {
public:
    /**
     * @brief Записывает блок команд в файл
     * @param commands Вектор команд для записи
     * @param timestamp Временная метка для формирования имени файла
     */
    void update(const std::vector<std::string>& commands, time_t timestamp) override {
        if (commands.empty()) return;
        
        std::string filename = "bulk" + std::to_string(timestamp) + ".log";
        std::ofstream file(filename);
        
        if (file.is_open()) {
            file << "bulk: ";
            for (size_t i = 0; i < commands.size(); ++i) {
                if (i != 0) file << ", ";
                file << commands[i];
            }
            file << std::endl;
        }
    }
};

/**
 * @brief Класс для управления подписками и уведомлениями наблюдателей
 * 
 * Реализует механизм подписки и уведомления в паттерне "Наблюдатель"
 */
class Observable {
public:
    /**
     * @brief Подписывает наблюдателя на уведомления
     * @param observer Указатель на наблюдателя для подписки
     */
    void subscribe(std::shared_ptr<IObserver> observer) {
        observers_.push_back(observer);
    }
    
    /**
     * @brief Уведомляет всех подписанных наблюдателей о новом блоке команд
     * @param commands Вектор команд для обработки
     * @param timestamp Временная метка первой команды в блоке
     */
    void notify(const std::vector<std::string>& commands, time_t timestamp) {
        for (auto& observer : observers_) {
            observer->update(commands, timestamp);
        }
    }
    
private:
    std::vector<std::shared_ptr<IObserver>> observers_; ///< Список подписанных наблюдателей
};

/**
 * @brief Класс для обработки и группировки команд в блоки
 * 
 * Обрабатывает входящие команды, формирует блоки согласно заданному размеру
 * и правилам для динамических блоков (в фигурных скобках)
 */
class CommandProcessor {
public:
    /**
     * @brief Конструктор обработчика команд
     * @param bulk_size Размер блока команд по умолчанию
     */
    CommandProcessor(size_t bulk_size = 3) 
        : bulk_size_(bulk_size), block_depth_(0), first_command_time_(0) {}
    
    /**
     * @brief Обрабатывает очередную команду
     * @param command Строка с командой для обработки
     */
    void process_command(const std::string& command) {
        if (command == "{") {
            // Начало динамического блока
            if (block_depth_ == 0) {
                flush_commands(); // Завершаем текущий блок перед началом нового
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
            // Обычная команда
            if (commands_.empty()) {
                // Фиксируем время первой команды в блоке
                first_command_time_ = std::chrono::system_clock::to_time_t(
                    std::chrono::system_clock::now());
            }
            commands_.push_back(command);
            
            // Автоматическое завершение блока при достижении размера (если не в динамическом блоке)
            if (block_depth_ == 0 && commands_.size() >= bulk_size_) {
                flush_commands();
            }
        }
    }
    
    /**
     * @brief Завершает обработку оставшихся команд
     * 
     * Вызывается при завершении ввода для обработки последнего блока команд
     */
    void flush_remaining() {
        if (block_depth_ == 0) {
            flush_commands(); // Завершаем текущий блок
        } else {
            commands_.clear(); // Игнорируем незавершенный динамический блок
        }
    }
    
    /**
     * @brief Устанавливает объект для уведомления о завершенных блоках
     * @param observable Указатель на объект для уведомлений
     */
    void set_observable(std::shared_ptr<Observable> observable) {
        observable_ = observable;
    }
    
private:
    /**
     * @brief Отправляет текущий блок команд наблюдателям и очищает буфер
     */
    void flush_commands() {
        if (!commands_.empty()) {
            observable_->notify(commands_, first_command_time_);
            commands_.clear();
        }
    }
    
    size_t bulk_size_;                  ///< Размер блока команд по умолчанию
    size_t block_depth_;                ///< Глубина вложенности динамических блоков
    std::vector<std::string> commands_; ///< Буфер накопленных команд
    time_t first_command_time_;         ///< Временная метка первой команды в текущем блоке
    std::shared_ptr<Observable> observable_; ///< Объект для уведомления о блоках
};
