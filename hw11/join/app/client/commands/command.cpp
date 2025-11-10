/**
 * @file command.cpp
 * @brief Реализация классов команд для клиента операций над множествами
 * @details 
 * Этот файл содержит реализацию команд для взаимодействия с сервером
 * операций над множествами. Каждая команда соответствует протоколу сервера.
 */

#include "command.h"
#include "../data/result_parser.h"
#include <iostream>

// InsertCommand implementation

/**
 * @brief Конструктор команды INSERT
 * @param table Имя таблицы ("A" или "B")
 * @param id Уникальный идентификатор записи
 * @param name Значение имени для вставки
 */
InsertCommand::InsertCommand(const std::string& table, int id, const std::string& name)
    : table_(table), id_(id), name_(name) {}

/**
 * @brief Выполняет команду INSERT на сервере
 * @param client Сетевой клиент для взаимодействия с сервером
 * @return true если команда выполнена успешно, false в случае ошибки
 * @details
 * Формирует команду INSERT в формате "INSERT table id name" и отправляет на сервер.
 * Обрабатывает ответ сервера и выводит результат операции.
 */
bool InsertCommand::execute(NetworkClient& client) {
    std::string command = "INSERT " + table_ + " " + std::to_string(id_) + " " + name_;
    std::string response = client.send_command(command);
    
    if (ResultParser::parse_simple_response(response)) {
        std::cout << "INSERT " << table_ << " " << id_ << " " << name_ << " - OK" << std::endl;
        return true;
    } else {
        std::string error = ResultParser::parse_error(response);
        std::cout << "INSERT " << table_ << " " << id_ << " " << name_ << " - FAILED: " << error << std::endl;
        return false;
    }
}

/**
 * @brief Возвращает имя команды
 * @return Строка "INSERT"
 */
std::string InsertCommand::get_name() const {
    return "INSERT";
}

// TruncateCommand implementation

/**
 * @brief Конструктор команды TRUNCATE
 * @param table Имя таблицы для очистки ("A" или "B")
 */
TruncateCommand::TruncateCommand(const std::string& table) : table_(table) {}

/**
 * @brief Выполняет команду TRUNCATE на сервере
 * @param client Сетевой клиент для взаимодействия с сервером
 * @return true если команда выполнена успешно, false в случае ошибки
 * @details
 * Формирует команду TRUNCATE в формате "TRUNCATE table" и отправляет на сервер.
 * Очищает указанную таблицу от всех записей.
 */
bool TruncateCommand::execute(NetworkClient& client) {
    std::string command = "TRUNCATE " + table_;
    std::string response = client.send_command(command);
    
    if (ResultParser::parse_simple_response(response)) {
        std::cout << "TRUNCATE " << table_ << " - OK" << std::endl;
        return true;
    } else {
        std::string error = ResultParser::parse_error(response);
        std::cout << "TRUNCATE " << table_ << " - FAILED: " << error << std::endl;
        return false;
    }
}

/**
 * @brief Возвращает имя команды
 * @return Строка "TRUNCATE"
 */
std::string TruncateCommand::get_name() const {
    return "TRUNCATE";
}

// IntersectionCommand implementation

/**
 * @brief Выполняет команду INTERSECTION на сервере
 * @param client Сетевой клиент для взаимодействия с сервером
 * @return true если команда выполнена успешно, false в случае ошибки
 * @details
 * Отправляет команду INTERSECTION и выводит результат пересечения множеств
 * из таблиц A и B. Результат содержит строки с одинаковыми ID из обеих таблиц.
 * Формат вывода:
 * id | A | B
 * ---+---+---
 * 3  | violation | proposal
 * 4  | quality | example
 * 5  | precision | lake
 */
bool IntersectionCommand::execute(NetworkClient& client) {
    std::string response = client.send_command("INTERSECTION");
    auto result = ResultParser::parse_intersection(response);
    
    if (result.success) {
        std::cout << "INTERSECTION result:" << std::endl;
        std::cout << "id | A | B" << std::endl;
        std::cout << "---+---+---" << std::endl;
        for (const auto& row : result.rows) {
            std::cout << row.id << " | " << row.column_a << " | " << row.column_b << std::endl;
        }
        std::cout << "OK" << std::endl;
        return true;
    } else {
        std::cout << "INTERSECTION - FAILED: " << result.error_message << std::endl;
        return false;
    }
}

/**
 * @brief Возвращает имя команды
 * @return Строка "INTERSECTION"
 */
std::string IntersectionCommand::get_name() const {
    return "INTERSECTION";
}

// SymmetricDifferenceCommand implementation

/**
 * @brief Выполняет команду SYMMETRIC_DIFFERENCE на сервере
 * @param client Сетевой клиент для взаимодействия с сервером
 * @return true если команда выполнена успешно, false в случае ошибки
 * @details
 * Отправляет команду SYMMETRIC_DIFFERENCE и выводит результат симметрической разности
 * множеств из таблиц A и B. Результат содержит строки, которые есть только в одной из таблиц.
 * Формат вывода:
 * id | A | B
 * ---+---+---
 * 0  | lean |
 * 1  | sweater |
 * 2  | frank |
 * 6  | | flour
 * 7  | | wonder
 * 8  | | selection
 */
bool SymmetricDifferenceCommand::execute(NetworkClient& client) {
    std::string response = client.send_command("SYMMETRIC_DIFFERENCE");
    auto result = ResultParser::parse_symmetric_difference(response);
    
    if (result.success) {
        std::cout << "SYMMETRIC DIFFERENCE result:" << std::endl;
        std::cout << "id | A | B" << std::endl;
        std::cout << "---+---+---" << std::endl;
        for (const auto& row : result.rows) {
            std::cout << row.id << " | " << row.column_a << " | " << row.column_b << std::endl;
        }
        std::cout << "OK" << std::endl;
        return true;
    } else {
        std::cout << "SYMMETRIC DIFFERENCE - FAILED: " << result.error_message << std::endl;
        return false;
    }
}

/**
 * @brief Возвращает имя команды
 * @return Строка "SYMMETRIC_DIFFERENCE"
 */
std::string SymmetricDifferenceCommand::get_name() const {
    return "SYMMETRIC_DIFFERENCE";
}
