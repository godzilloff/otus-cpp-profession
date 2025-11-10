/**
 * @file command_handler.cpp
 * @brief Реализация класса CommandHandler для обработки команд сервера
 */

#include "command_handler.h"
#include <iostream>

/**
 * @brief Конструктор обработчика команд
 * @param factory Указатель на фабрику команд для создания объектов команд
 * @details Принимает фабрику команд и перемещает ее в член класса
 */
CommandHandler::CommandHandler(std::unique_ptr<CommandFactory> factory) 
    : factory_(std::move(factory)) {}

/**
 * @brief Обрабатывает команду, полученную от клиента
 * @param command_line Строка с командой от клиента
 * @return Строка с результатом выполнения команды или сообщением об ошибке
 * @details 
 * - Создает команду через фабрику команд
 * - Выполняет команду и возвращает результат
 * - В случае исключения возвращает сообщение об ошибке с префиксом "ERR"
 * 
 * Примеры возвращаемых значений:
 * - "OK" - команда выполнена успешно
 * - "ERR duplicate 0" - ошибка дублирования ID
 * - "ERR unknown command" - неизвестная команда
 * 
 * @throws Может пробрасывать исключения из фабрики команд или выполнения команд
 */
std::string CommandHandler::handle_command(const std::string& command_line) {
    try {
        // Создаем команду через фабрику
        auto command = factory_->create_command(command_line);
        // Выполняем команду и возвращаем результат
        return command->execute();
    } catch (const std::exception& e) {
        // Возвращаем сообщение об ошибке с префиксом ERR
        return "ERR " + std::string(e.what());
    }
}
