/**
 * @file command.cpp
 * @brief Реализация классов команд для сервера операций над множествами
 * @details Содержит реализацию методов выполнения команд INSERT, TRUNCATE, INTERSECTION и SYMMETRIC_DIFFERENCE
 */

#include "command.h"
#include <sstream>

/**
 * @brief Выполняет команду INSERT - вставка данных в таблицу
 * @return "OK" при успешном выполнении, "ERR duplicate <id>" при дублировании ID
 * @details Пытается вставить запись с указанным ID и именем в указанную таблицу.
 * Если запись с таким ID уже существует, возвращает ошибку.
 */
std::string InsertCommand::execute() {
    if (!repo_->insert(table_, id_, name_)) {
        return "ERR duplicate " + std::to_string(id_);
    }
    return "OK";
}

/**
 * @brief Выполняет команду TRUNCATE - очистка таблицы
 * @return "OK" при успешном выполнении, "ERR truncate failed" при ошибке
 * @details Полностью очищает указанную таблицу, удаляя все записи.
 * Проверяет корректность имени таблицы (должна быть "A" или "B").
 */
std::string TruncateCommand::execute() {
    if (!repo_->truncate(table_)) {
        return "ERR truncate failed";
    }
    return "OK";
}

/**
 * @brief Выполняет команду INTERSECTION - пересечение множеств
 * @return Многострочный результат в формате CSV с последней строкой "OK"
 * @details Возвращает записи, присутствующие в обеих таблицах A и B с одинаковыми ID.
 * Формат вывода: id,значение_из_A,значение_из_B
 * Результаты сортируются по возрастанию ID.
 */
std::string IntersectionCommand::execute() {
    auto results = repo_->intersection();
    std::ostringstream oss;
    
    for (const auto& row : results) {
        oss << row.id << "," << row.a << "," << row.b << "\n";
    }
    
    oss << "OK";
    return oss.str();
}

/**
 * @brief Выполняет команду SYMMETRIC_DIFFERENCE - симметрическая разность множеств
 * @return Многострочный результат в формате CSV с последней строкой "OK"
 * @details Возвращает записи, присутствующие только в одной из таблиц (A или B).
 * Формат вывода: id,значение_из_A,значение_из_B
 * Для записей только из A: значение_из_B будет пустым
 * Для записей только из B: значение_из_A будет пустым
 * Результаты сортируются по возрастанию ID.
 */
std::string SymmetricDifferenceCommand::execute() {
    auto results = repo_->symmetric_difference();
    std::ostringstream oss;
    
    for (const auto& row : results) {
        oss << row.id << "," << row.a << "," << row.b << "\n";
    }
    
    oss << "OK";
    return oss.str();
}
