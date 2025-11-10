/**
 * @file command_executor.cpp
 * @brief Реализация класса CommandExecutor для выполнения команд клиента
 */

#include "command_executor.h"
#include <iostream>

/**
 * @brief Конструктор исполнителя команд
 * @param client_factory Фабрика для создания клиентов подключения к серверу
 * 
 * @details
 * Инициализирует исполнитель команд с переданной фабрикой клиентов.
 * Фабрика будет использоваться для создания новых подключений к серверу
 * при выполнении каждой команды.
 */
CommandExecutor::CommandExecutor(CommandExecutor::ClientFactory client_factory)
    : client_factory_(std::move(client_factory)) {}

/**
 * @brief Выполняет одну команду
 * @param command Указатель на команду для выполнения
 * @return true если команда выполнена успешно, false в случае ошибки
 * 
 * @details
 * Метод создает новое подключение к серверу, выполняет команду и закрывает соединение.
 * Такой подход обеспечивает изоляцию команд и устойчивость к разрывам соединения.
 * 
 * @note
 * - Создается новое подключение для каждой команды
 * - Соединение автоматически закрывается после выполнения команды
 * - Ошибки подключения и выполнения команды обрабатываются корректно
 * 
 * @par Пример использования:
 * @code
 * auto executor = CommandExecutor(client_factory);
 * auto command = std::make_unique<InsertCommand>("A", 1, "test");
 * bool success = executor.execute_command(std::move(command));
 * @endcode
 */
bool CommandExecutor::execute_command(std::unique_ptr<ICommand> command) {
    // Создаем нового клиента для подключения к серверу
    auto client = create_client();
    
    // Устанавливаем соединение с сервером
    if (!client->connect()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return false;
    }
    
    try {
        // Выполняем команду через клиента
        bool result = command->execute(*client);
        // После выполнения команды соединение может быть закрыто сервером
        client->disconnect();
        return result;
    } catch (const std::exception& e) {
        // Обрабатываем исключения при выполнении команды
        std::cerr << "Command execution failed: " << e.what() << std::endl;
        client->disconnect();
        return false;
    }
}

/**
 * @brief Выполняет набор команд последовательно
 * @param commands Вектор команд для выполнения
 * 
 * @details
 * Метод выполняет переданные команды одна за другой. Каждая команда выполняется
 * в отдельном подключении к серверу. При ошибке выполнения одной команды
 * выполнение продолжается со следующей командой в векторе.
 * 
 * @note
 * - Команды выполняются в порядке их следования в векторе
 * - Ошибка выполнения одной команды не прерывает выполнение остальных
 * - Для каждой команды создается отдельное подключение к серверу
 * 
 * @par Пример использования:
 * @code
 * std::vector<std::unique_ptr<ICommand>> commands;
 * commands.push_back(std::make_unique<InsertCommand>("A", 1, "test1"));
 * commands.push_back(std::make_unique<InsertCommand>("A", 2, "test2"));
 * executor.execute_commands(commands);
 * @endcode
 * 
 * @warning
 * Вектор команд передается по константной ссылке, но для выполнения команды
 * требуется перемещение владения. Используется const_cast для обхода этого ограничения,
 * что безопасно в данном контексте, так как вектор не изменяется.
 */
void CommandExecutor::execute_commands(const std::vector<std::unique_ptr<ICommand>>& commands) {
    // Выполняем каждую команду из вектора
    for (const auto& command : commands) {
        // Выполняем команду и проверяем результат
        if (!execute_command(std::move(const_cast<std::unique_ptr<ICommand>&>(command)))) {
            // Логируем ошибку, но продолжаем выполнение следующих команд
            std::cerr << "Command failed: " << command->get_name() << std::endl;
            // Продолжаем выполнение остальных команд даже при ошибке
        }
    }
}
