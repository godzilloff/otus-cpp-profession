#pragma once
#include <memory>
#include <string>
#include "../commands/command_factory.h"

/**
 * @class CommandHandler
 * @brief Обработчик команд для сервера операций над множествами
 * 
 * Класс отвечает за обработку входящих команд от клиента.
 * Использует фабрику команд для создания соответствующих объектов команд
 * и выполнения их бизнес-логики.
 */
class CommandHandler {
public:
    /**
     * @brief Конструктор обработчика команд
     * @param factory Указатель на фабрику команд для создания объектов команд
     * @details Принимает владение фабрикой команд через std::unique_ptr
     */
    explicit CommandHandler(std::unique_ptr<CommandFactory> factory);
    
    /**
     * @brief Обрабатывает команду от клиента
     * @param command_line Строка с командой от клиента
     * @return Результат выполнения команды в виде строки
     * @details 
     * - Парсит команду и создает соответствующий объект команды через фабрику
     * - Выполняет команду и возвращает результат
     * - В случае ошибки возвращает строку с префиксом "ERR"
     * 
     * @note Форматы команд:
     * - INSERT table id name
     * - TRUNCATE table  
     * - INTERSECTION
     * - SYMMETRIC_DIFFERENCE
     * 
     * @example
     * handle_command("INSERT A 0 lean") -> "OK"
     * handle_command("INTERSECTION") -> "3,violation,proposal\n4,quality,example\n5,precision,lake\nOK"
     */
    std::string handle_command(const std::string& command_line);

private:
    /// @brief Фабрика команд для создания объектов команд
    std::unique_ptr<CommandFactory> factory_;
};
