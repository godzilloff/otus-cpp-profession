#pragma once
#include <memory>
#include <string>
#include <vector>
#include "command.h"

/**
 * @class CommandFactory
 * @brief Фабрика для создания объектов команд на основе текстового ввода
 * 
 * Класс отвечает за разбор текстовых команд протокола и создание соответствующих
 * объектов команд для выполнения операций над множествами.
 */
class CommandFactory {
public:
    /**
     * @brief Конструктор фабрики команд
     * @param repo Репозиторий для работы с таблицами данных
     */
    explicit CommandFactory(std::shared_ptr<ITableRepository> repo);
    
    /**
     * @brief Создает команду на основе текстовой строки
     * @param command_line Текстовая команда в формате протокола
     * @return Уникальный указатель на объект команды
     * @throws std::invalid_argument Если команда некорректна или неизвестна
     * 
     * Поддерживаемые команды:
     * - INSERT table id name
     * - TRUNCATE table
     * - INTERSECTION
     * - SYMMETRIC_DIFFERENCE
     */
    std::unique_ptr<ICommand> create_command(const std::string& command_line);

private:
    std::shared_ptr<ITableRepository> repo_; ///< Репозиторий для доступа к данным таблиц
    
    /**
     * @brief Разбирает команду INSERT
     * @param tokens Токенизированные части команды
     * @return Команда для вставки данных
     * @throws std::invalid_argument Если неверное количество аргументов
     */
    std::unique_ptr<ICommand> parse_insert(const std::vector<std::string>& tokens);
    
    /**
     * @brief Разбирает команду TRUNCATE
     * @param tokens Токенизированные части команды
     * @return Команда для очистки таблицы
     * @throws std::invalid_argument Если неверное количество аргументов
     */
    std::unique_ptr<ICommand> parse_truncate(const std::vector<std::string>& tokens);
    
    /**
     * @brief Разбивает строку команды на токены
     * @param command_line Исходная строка команды
     * @return Вектор токенов (отдельных слов команды)
     * 
     * Разделителем считается пробел. Имя в команде INSERT может содержать пробелы
     * и будет собрано из оставшихся токенов.
     */
    std::vector<std::string> tokenize(const std::string& command_line);
};
