/**
 * @file repository.h
 * @brief Заголовочный файл репозитория для операций с таблицами базы данных
 */

#pragma once
#include "database.h"
#include <vector>
#include <string>

/**
 * @brief Структура для представления строки результата операций с таблицами
 */
struct ResultRow {
    int id;             ///< Уникальный идентификатор записи
    std::string a;      ///< Значение из таблицы A (может быть пустым)
    std::string b;      ///< Значение из таблицы B (может быть пустым)
};

/**
 * @brief Интерфейс репозитория для операций с таблицами A и B
 */
class ITableRepository {
public:
    virtual ~ITableRepository() = default;
    
    /**
     * @brief Вставка записи в указанную таблицу
     * @param table Название таблицы ("A" или "B")
     * @param id Уникальный идентификатор
     * @param name Значение имени
     * @return true при успешной вставке, false при ошибке
     */
    virtual bool insert(const std::string& table, int id, const std::string& name) = 0;
    
    /**
     * @brief Очистка указанной таблицы
     * @param table Название таблицы ("A" или "B")
     * @return true при успешной очистке, false при ошибке
     */
    virtual bool truncate(const std::string& table) = 0;
    
    /**
     * @brief Операция пересечения таблиц A и B
     * @return Вектор записей, присутствующих в обеих таблицах
     */
    virtual std::vector<ResultRow> intersection() = 0;
    
    /**
     * @brief Операция симметрической разности таблиц A и B
     * @return Вектор записей, присутствующих только в одной таблице
     */
    virtual std::vector<ResultRow> symmetric_difference() = 0;
};

/**
 * @brief Реализация репозитория для работы с таблицами базы данных
 * 
 * @details Класс предоставляет методы для выполнения операций CRUD
 * и операций над множествами для таблиц A и B.
 */
class TableRepository : public ITableRepository {
public:
    /**
     * @brief Конструктор репозитория
     * @param db Объект базы данных для выполнения запросов
     */
    explicit TableRepository(std::shared_ptr<Database> db);
    
    bool insert(const std::string& table, int id, const std::string& name) override;
    bool truncate(const std::string& table) override;
    std::vector<ResultRow> intersection() override;
    std::vector<ResultRow> symmetric_difference() override;

private:
    /**
     * @brief Выполнение JOIN запроса к базе данных
     * @param sql SQL запрос для выполнения
     * @return Результат запроса в виде вектора строк
     */
    std::vector<ResultRow> execute_join_query(const std::string& sql);
    
    std::shared_ptr<Database> db_;  ///< Указатель на объект базы данных
};
