/**
 * @file database.cpp
 * @brief Реализация класса Database для работы с SQLite
 */

#include "database.h"
#include <stdexcept>

/**
 * @brief Конструктор класса Database
 * @param connection_string Строка подключения к базе данных
 * @throw std::runtime_error Если не удалось открыть базу данных
 * 
 * @details
 * Создает подключение к SQLite базе данных и инициализирует таблицы.
 * Для хранения данных используется in-memory база по умолчанию.
 */
Database::Database(const std::string& connection_string) {
    int rc = sqlite3_open(connection_string.c_str(), &db_);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(db_)));
    }
    create_tables();
}

/**
 * @brief Деструктор класса Database
 * 
 * @details
 * Корректно закрывает соединение с базой данных и освобождает ресурсы.
 * Гарантирует безопасное завершение работы с SQLite.
 */
Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
    }
}

/**
 * @brief Создает таблицы A и B в базе данных
 * @throw std::runtime_error Если не удалось создать таблицы
 * 
 * @details
 * Создает две таблицы идентичной структуры:
 * - Таблица A: id (INTEGER PRIMARY KEY), name (TEXT NOT NULL)
 * - Таблица B: id (INTEGER PRIMARY KEY), name (TEXT NOT NULL)
 * 
 * Таблицы используются для хранения множеств и выполнения операций
 * пересечения и симметрической разности.
 */
void Database::create_tables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS A (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL
        );
        
        CREATE TABLE IF NOT EXISTS B (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL
        );
    )";
    
    char* errmsg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        std::string error = errmsg;
        sqlite3_free(errmsg);
        throw std::runtime_error("Failed to create tables: " + error);
    }
}
