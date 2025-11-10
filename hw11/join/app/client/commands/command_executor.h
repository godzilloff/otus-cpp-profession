#pragma once
#include "command.h"
#include "../data/result_parser.h"
#include <memory>
#include <vector>
#include <functional>

/**
 * @class CommandExecutor
 * @brief Исполнитель команд для взаимодействия с сервером операций над множествами
 * 
 * Класс отвечает за выполнение команд через сетевое соединение с сервером.
 * Реализует паттерн "Команда" для инкапсуляции запросов к серверу.
 * Поддерживает как одиночное выполнение команд, так и пакетное выполнение.
 */
class CommandExecutor {
public:
    /// Тип фабрики для создания сетевых клиентов
    using ClientFactory = std::function<std::unique_ptr<NetworkClient>()>;
    
    /**
     * @brief Конструктор исполнителя команд
     * @param client_factory Фабрика для создания клиентов сетевого соединения
     * 
     * Принимает фабричную функцию, которая будет создавать новые экземпляры
     * NetworkClient для каждого выполнения команды, обеспечивая изоляцию соединений.
     */
    explicit CommandExecutor(ClientFactory client_factory);
    
    /**
     * @brief Выполняет одиночную команду
     * @param command Указатель на команду для выполнения
     * @return true если команда выполнена успешно, false в случае ошибки
     * 
     * Создает новое сетевое соединение, выполняет команду и закрывает соединение.
     * Автоматически обрабатывает ошибки соединения и выполнения команды.
     */
    bool execute_command(std::unique_ptr<ICommand> command);
    
    /**
     * @brief Выполняет набор команд последовательно
     * @param commands Вектор команд для выполнения
     * 
     * Выполняет команды в порядке их следования в векторе.
     * Каждая команда выполняется в отдельном сетевом соединении.
     * Продолжает выполнение даже при ошибках в отдельных командах.
     */
    void execute_commands(const std::vector<std::unique_ptr<ICommand>>& commands);
    
private:
    /// Фабрика для создания клиентов сетевого соединения
    ClientFactory client_factory_;
    
    /**
     * @brief Создает новый экземпляр сетевого клиента
     * @return Уникальный указатель на созданный NetworkClient
     * 
     * Внутренний метод для инкапсуляции создания сетевых клиентов
     * через предоставленную фабричную функцию.
     */
    std::unique_ptr<NetworkClient> create_client() const {
        return client_factory_();
    }
};
