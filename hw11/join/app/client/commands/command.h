#pragma once
#include <string>
#include <memory>
#include "../network/client.h"
#include "../data/table_data.h"

/**
 * @brief Базовый интерфейс для всех команд клиента
 * 
 * Определяет контракт для выполнения команд и получения их имен.
 * Реализует принцип Dependency Inversion из SOLID.
 */
class ICommand {
public:
    virtual ~ICommand() = default;
    
    /**
     * @brief Выполняет команду через сетевого клиента
     * @param client Ссылка на сетевой клиент для отправки команд
     * @return true если команда выполнена успешно, false в случае ошибки
     */
    virtual bool execute(NetworkClient& client) = 0;
    
    /**
     * @brief Возвращает имя команды
     * @return Строковое имя команды (INSERT, TRUNCATE, etc.)
     */
    virtual std::string get_name() const = 0;
};

/**
 * @brief Команда INSERT для добавления данных в таблицу
 * 
 * Отправляет серверу команду INSERT table id name для добавления записи
 * в указанную таблицу. ID является первичным ключом и не может дублироваться.
 */
class InsertCommand : public ICommand {
public:
    /**
     * @brief Конструктор команды INSERT
     * @param table Имя таблицы ("A" или "B")
     * @param id Уникальный идентификатор записи
     * @param name Значение имени для записи
     */
    InsertCommand(const std::string& table, int id, const std::string& name);
    
    /**
     * @brief Выполняет команду INSERT
     * @param client Сетевой клиент для отправки команды
     * @return true если запись добавлена успешно, false при ошибке (дубликат ID)
     */
    bool execute(NetworkClient& client) override;
    
    /**
     * @brief Возвращает имя команды
     * @return "INSERT"
     */
    std::string get_name() const override;

private:
    std::string table_;  ///< Имя целевой таблицы (A или B)
    int id_;             ///< Уникальный идентификатор записи
    std::string name_;   ///< Значение поля name для вставки
};

/**
 * @brief Команда TRUNCATE для очистки таблицы
 * 
 * Отправляет серверу команду TRUNCATE table для полной очистки
 * указанной таблицы. Удаляет все записи из таблицы.
 */
class TruncateCommand : public ICommand {
public:
    /**
     * @brief Конструктор команды TRUNCATE
     * @param table Имя таблицы для очистки ("A" или "B")
     */
    explicit TruncateCommand(const std::string& table);
    
    /**
     * @brief Выполняет команду TRUNCATE
     * @param client Сетевой клиент для отправки команды
     * @return true если таблица очищена успешно, false при ошибке
     */
    bool execute(NetworkClient& client) override;
    
    /**
     * @brief Возвращает имя команды
     * @return "TRUNCATE"
     */
    std::string get_name() const override;

private:
    std::string table_;  ///< Имя таблицы для очистки (A или B)
};

/**
 * @brief Команда INTERSECTION для получения пересечения множеств
 * 
 * Отправляет серверу команду INTERSECTION и выводит результат
 * пересечения данных из таблиц A и B по полю id.
 * 
 * Результат содержит строки, присутствующие в обеих таблицах
 * с одинаковым id, с именами из обеих таблиц.
 */
class IntersectionCommand : public ICommand {
public:
    IntersectionCommand() = default;
    
    /**
     * @brief Выполняет команду INTERSECTION
     * @param client Сетевой клиент для отправки команды
     * @return true если операция выполнена успешно, false при ошибке
     * 
     * Формат вывода:
     * id | A | B
     * 3 | violation | proposal
     * 4 | quality | example
     * 5 | precision | lake
     */
    bool execute(NetworkClient& client) override;
    
    /**
     * @brief Возвращает имя команды
     * @return "INTERSECTION"
     */
    std::string get_name() const override;
};

/**
 * @brief Команда SYMMETRIC_DIFFERENCE для получения симметрической разности
 * 
 * Отправляет серверу команду SYMMETRIC_DIFFERENCE и выводит результат
 * симметрической разности данных из таблиц A и B.
 * 
 * Результат содержит строки, присутствующие только в одной из таблиц
 * (либо в A, либо в B), отсортированные по id.
 */
class SymmetricDifferenceCommand : public ICommand {
public:
    SymmetricDifferenceCommand() = default;
    
    /**
     * @brief Выполняет команду SYMMETRIC_DIFFERENCE
     * @param client Сетевой клиент для отправки команды
     * @return true если операция выполнена успешно, false при ошибке
     * 
     * Формат вывода:
     * id | A | B
     * 0 | lean |
     * 1 | sweater |
     * 2 | frank |
     * 6 | | flour
     * 7 | | wonder
     * 8 | | selection
     */
    bool execute(NetworkClient& client) override;
    
    /**
     * @brief Возвращает имя команды
     * @return "SYMMETRIC_DIFFERENCE"
     */
    std::string get_name() const override;
};
