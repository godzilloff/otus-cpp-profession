/**
 * @file command_factory.cpp
 * @brief Реализация фабрики команд для обработки запросов к серверу множеств
 * @details 
 * Фабрика преобразует текстовые команды протокола в объекты команд,
 * которые затем выполняются над репозиторием таблиц.
 */

#include "command_factory.h"
#include <sstream>
#include <stdexcept>

/**
 * @brief Конструктор фабрики команд
 * @param repo Указатель на репозиторий таблиц для выполнения операций
 * @details 
 * Репозиторий используется всеми создаваемыми командами для доступа к данным
 */
CommandFactory::CommandFactory(std::shared_ptr<ITableRepository> repo) 
    : repo_(std::move(repo)) {}

/**
 * @brief Создает команду на основе текстовой строки
 * @param command_line Текстовая команда в формате протокола
 * @return Уникальный указатель на объект команды
 * @throws std::invalid_argument Если команда пустая, неизвестная или имеет неверный формат
 * @details
 * Команда парсится на токены, определяется тип команды и создается соответствующий объект.
 * Поддерживаемые команды: INSERT, TRUNCATE, INTERSECTION, SYMMETRIC_DIFFERENCE
 * 
 * Примеры команд:
 * - "INSERT A 0 lean"
 * - "TRUNCATE A" 
 * - "INTERSECTION"
 * - "SYMMETRIC_DIFFERENCE"
 */
std::unique_ptr<ICommand> CommandFactory::create_command(const std::string& command_line) {
    auto tokens = tokenize(command_line);
    
    if (tokens.empty()) {
        throw std::invalid_argument("Empty command");
    }
    
    const std::string& command_name = tokens[0];
    
    if (command_name == "INSERT") {
        return parse_insert(tokens);
    } else if (command_name == "TRUNCATE") {
        return parse_truncate(tokens);
    } else if (command_name == "INTERSECTION") {
        if (tokens.size() != 1) {
            throw std::invalid_argument("INTERSECTION command takes no arguments");
        }
        return std::make_unique<IntersectionCommand>(repo_);
    } else if (command_name == "SYMMETRIC_DIFFERENCE") {
        if (tokens.size() != 1) {
            throw std::invalid_argument("SYMMETRIC_DIFFERENCE command takes no arguments");
        }
        return std::make_unique<SymmetricDifferenceCommand>(repo_);
    } else {
        throw std::invalid_argument("Unknown command: " + command_name);
    }
}

/**
 * @brief Парсит команду INSERT и создает соответствующий объект
 * @param tokens Вектор токенов команды
 * @return Уникальный указатель на команду INSERT
 * @throws std::invalid_argument Если недостаточно параметров
 * @details
 * Формат команды: INSERT table id name
 * - table: имя таблицы (A или B)
 * - id: целочисленный идентификатор
 * - name: строковое имя (может содержать пробелы)
 * 
 * Пример: "INSERT A 0 lean" -> InsertCommand("A", 0, "lean")
 */
std::unique_ptr<ICommand> CommandFactory::parse_insert(const std::vector<std::string>& tokens) {
    if (tokens.size() < 4) {
        throw std::invalid_argument("INSERT command requires table, id and name");
    }
    
    std::string table = tokens[1];
    int id = std::stoi(tokens[2]);
    
    // Собираем имя из оставшихся токенов (может содержать пробелы)
    std::string name;
    for (size_t i = 3; i < tokens.size(); ++i) {
        if (i > 3) name += " ";
        name += tokens[i];
    }
    
    return std::make_unique<InsertCommand>(repo_, table, id, name);
}

/**
 * @brief Парсит команду TRUNCATE и создает соответствующий объект
 * @param tokens Вектор токенов команды
 * @return Уникальный указатель на команду TRUNCATE
 * @throws std::invalid_argument Если недостаточно параметров
 * @details
 * Формат команды: TRUNCATE table
 * - table: имя таблицы (A или B)
 * 
 * Пример: "TRUNCATE A" -> TruncateCommand("A")
 */
std::unique_ptr<ICommand> CommandFactory::parse_truncate(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2) {
        throw std::invalid_argument("TRUNCATE command requires table name");
    }
    
    std::string table = tokens[1];
    return std::make_unique<TruncateCommand>(repo_, table);
}

/**
 * @brief Разбивает строку команды на токены
 * @param command_line Исходная строка команды
 * @return Вектор строк-токенов
 * @details
 * Использует stringstream для разбиения строки по пробелам.
 * Первый токен - имя команды, последующие - параметры.
 * 
 * Пример: "INSERT A 0 lean" -> ["INSERT", "A", "0", "lean"]
 */
std::vector<std::string> CommandFactory::tokenize(const std::string& command_line) {
    std::vector<std::string> tokens;
    std::istringstream iss(command_line);
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}
