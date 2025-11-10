/**
 * @file command.h
 * @brief Заголовочный файл, содержащий интерфейс и реализации команд для работы с сервером множеств
 */

#pragma once
#include <string>
#include <memory>
#include "../database/repository.h"

/**
 * @brief Базовый интерфейс для всех команд
 * 
 * Определяет контракт для выполнения операций над таблицами A и B
 */
class ICommand {
public:
    virtual ~ICommand() = default;
    
    /**
     * @brief Выполняет команду
     * @return Результат выполнения команды в виде строки
     * 
     * Для успешного выполнения возвращает "OK", для ошибок - "ERR <описание>"
     */
    virtual std::string execute() = 0;
};

/**
 * @brief Команда INSERT для добавления данных в таблицу
 * 
 * Добавляет запись с указанным ID и именем в таблицу A или B
 * Формат команды: INSERT <table> <id> <name>
 */
class InsertCommand : public ICommand {
public:
    /**
     * @brief Конструктор команды INSERT
     * @param repo Репозиторий для работы с данными
     * @param table Имя таблицы ("A" или "B")
     * @param id Уникальный идентификатор записи
     * @param name Имя записи
     */
    InsertCommand(std::shared_ptr<ITableRepository> repo, 
                  std::string table, int id, std::string name)
        : repo_(std::move(repo)), table_(std::move(table)), 
          id_(id), name_(std::move(name)) {}
    
    std::string execute() override;

private:
    std::shared_ptr<ITableRepository> repo_; ///< Репозиторий для доступа к данным
    std::string table_; ///< Имя таблицы ("A" или "B")
    int id_;           ///< Уникальный идентификатор записи
    std::string name_; ///< Имя записи
};

/**
 * @brief Команда TRUNCATE для очистки таблицы
 * 
 * Удаляет все записи из указанной таблицы
 * Формат команды: TRUNCATE <table>
 */
class TruncateCommand : public ICommand {
public:
    /**
     * @brief Конструктор команды TRUNCATE
     * @param repo Репозиторий для работы с данными
     * @param table Имя таблицы ("A" или "B")
     */
    TruncateCommand(std::shared_ptr<ITableRepository> repo, std::string table)
        : repo_(std::move(repo)), table_(std::move(table)) {}
    
    std::string execute() override;

private:
    std::shared_ptr<ITableRepository> repo_; ///< Репозиторий для доступа к данным
    std::string table_; ///< Имя таблицы ("A" или "B")
};

/**
 * @brief Команда INTERSECTION для получения пересечения множеств
 * 
 * Возвращает записи, присутствующие в обеих таблицах A и B с одинаковыми ID
 * Формат команды: INTERSECTION
 * 
 * @return Результат в формате CSV: id,значение_из_A,значение_из_B
 */
class IntersectionCommand : public ICommand {
public:
    /**
     * @brief Конструктор команды INTERSECTION
     * @param repo Репозиторий для работы с данными
     */
    explicit IntersectionCommand(std::shared_ptr<ITableRepository> repo)
        : repo_(std::move(repo)) {}
    
    std::string execute() override;

private:
    std::shared_ptr<ITableRepository> repo_; ///< Репозиторий для доступа к данным
};

/**
 * @brief Команда SYMMETRIC_DIFFERENCE для получения симметрической разности
 * 
 * Возвращает записи, присутствующие только в одной из таблиц (A или B)
 * Формат команды: SYMMETRIC_DIFFERENCE
 * 
 * @return Результат в формате CSV: id,значение_из_A,значение_из_B
 *         Для записей только из A: значение_из_B пустое
 *         Для записей только из B: значение_из_A пустое
 */
class SymmetricDifferenceCommand : public ICommand {
public:
    /**
     * @brief Конструктор команды SYMMETRIC_DIFFERENCE
     * @param repo Репозиторий для работы с данными
     */
    explicit SymmetricDifferenceCommand(std::shared_ptr<ITableRepository> repo)
        : repo_(std::move(repo)) {}
    
    std::string execute() override;

private:
    std::shared_ptr<ITableRepository> repo_; ///< Репозиторий для доступа к данным
};
