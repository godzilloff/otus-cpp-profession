#pragma once
#include <sqlite3.h>
#include <memory>
#include <string>

/**
 * @class Database
 * @brief Класс для работы с базой данных SQLite
 * 
 * Обеспечивает создание и управление соединением с базой данных,
 * создание таблиц A и B для хранения множеств.
 * Использует in-memory базу данных по умолчанию.
 */
class Database {
public:
    /**
     * @brief Конструктор класса Database
     * @param connection_string Строка подключения к базе данных (по умолчанию in-memory)
     * @throw std::runtime_error Если не удалось открыть базу данных
     * 
     * Создает соединение с SQLite базой данных и инициализирует таблицы.
     * По умолчанию использует in-memory базу для хранения данных в оперативной памяти.
     */
    Database(const std::string& connection_string = ":memory:");
    
    /**
     * @brief Деструктор класса Database
     * 
     * Корректно закрывает соединение с базой данных и освобождает ресурсы.
     */
    ~Database();
    
    // Запрещаем копирование
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    /**
     * @brief Получить указатель на соединение с базой данных
     * @return sqlite3* Указатель на объект SQLite базы данных
     * 
     * Предоставляет прямой доступ к низкоуровневому соединению SQLite
     * для выполнения SQL-запросов.
     */
    sqlite3* get_handle() { return db_; }
    
private:
    /**
     * @brief Создает таблицы A и B в базе данных
     * @throw std::runtime_error Если не удалось создать таблицы
     * 
     * Создает две таблицы идентичной структуры:
     * - Таблица A: id INTEGER PRIMARY KEY, name TEXT NOT NULL
     * - Таблица B: id INTEGER PRIMARY KEY, name TEXT NOT NULL
     * id является первичным ключом и не может дублироваться в одной таблице.
     */
    void create_tables();
    
    /**
     * @brief Указатель на объект базы данных SQLite
     * 
     * Хранит соединение с базой данных SQLite. Используется для выполнения
     * всех SQL-операций: вставки, удаления, выборки данных.
     */
    sqlite3* db_;
};
