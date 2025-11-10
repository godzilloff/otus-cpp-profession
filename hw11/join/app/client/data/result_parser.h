#pragma once
#include "table_data.h"
#include <string>
#include <vector>
#include <sstream>

/**
 * @class ResultParser
 * @brief Парсер ответов от сервера для операций над множествами
 * 
 * Класс предоставляет статические методы для разбора ответов сервера
 * на команды INTERSECTION и SYMMETRIC_DIFFERENCE, а также для обработки
 * простых ответов и ошибок.
 */
class ResultParser {
public:
    /**
     * @brief Разбор ответа на команду INTERSECTION
     * @param response Ответ от сервера в формате CSV
     * @return OperationResult Структура с результатами пересечения
     * 
     * Пример ответа:
     * 3,violation,proposal
     * 4,quality,example
     * 5,precision,lake
     * OK
     */
    static OperationResult parse_intersection(const std::string& response) {
        return parse_multi_line_result(response);
    }
    
    /**
     * @brief Разбор ответа на команду SYMMETRIC_DIFFERENCE
     * @param response Ответ от сервера в формате CSV
     * @return OperationResult Структура с результатами симметрической разности
     * 
     * Пример ответа:
     * 0,lean,
     * 1,sweater,
     * 2,frank,
     * 6,,flour
     * 7,,wonder
     * 8,,selection
     * OK
     */
    static OperationResult parse_symmetric_difference(const std::string& response) {
        return parse_multi_line_result(response);
    }
    
    /**
     * @brief Проверка простого успешного ответа
     * @param response Ответ от сервера
     * @return true Если ответ содержит "OK", иначе false
     * 
     * Используется для команд INSERT и TRUNCATE
     */
    static bool parse_simple_response(const std::string& response) {
        return response.find("OK") != std::string::npos;
    }
    
    /**
     * @brief Извлечение сообщения об ошибке из ответа
     * @param response Ответ от сервера
     * @return std::string Текст ошибки или пустая строка если ошибки нет
     * 
     * Примеры ошибок:
     * - "ERR duplicate 0"
     * - "ERR invalid table name"
     */
    static std::string parse_error(const std::string& response) {
        if (response.find("ERR") == 0) {
            return response.substr(4); // Удаляем "ERR "
        }
        return "";
    }

private:
    /**
     * @brief Разбор многострочного ответа от сервера
     * @param response Многострочный ответ в CSV формате
     * @return OperationResult Структура с разобранными данными
     * 
     * Обрабатывает ответы формата:
     * - Каждая строка содержит данные в формате CSV
     * - Последняя строка "OK" или "ERR message"
     * - Пустые значения обозначаются отсутствием данных между запятых
     */
    static OperationResult parse_multi_line_result(const std::string& response) {
        OperationResult result;
        std::istringstream iss(response);
        std::string line;
        
        while (std::getline(iss, line)) {
            if (line == "OK") {
                result.success = true;
                break;
            }
            
            if (line.find("ERR") == 0) {
                result.success = false;
                result.error_message = line.substr(4);
                break;
            }
            
            // Парсим CSV строку: id,значение_а,значение_б
            auto row = parse_csv_line(line);
            if (row.id != -1) {
                result.rows.push_back(row);
            }
        }
        
        return result;
    }
    
    /**
     * @brief Разбор одной CSV строки
     * @param line Строка в формате CSV
     * @return OperationResult::ResultRow Разобранная строка данных
     * 
     * Формат строки: id,column_a,column_b
     * Примеры:
     * - "3,violation,proposal" - полная строка
     * - "0,lean," - отсутствует значение в колонке B
     * - "6,,flour" - отсутствует значение в колонке A
     */
    static OperationResult::ResultRow parse_csv_line(const std::string& line) {
        OperationResult::ResultRow row{-1, "", ""};
        std::istringstream line_stream(line);
        std::string token;
        
        // Парсим ID
        if (std::getline(line_stream, token, ',')) {
            try {
                row.id = std::stoi(token);
            } catch (...) {
                return row; // Некорректный ID
            }
        }
        
        // Парсим Колонку A
        if (std::getline(line_stream, token, ',')) {
            row.column_a = token;
        }
        
        // Парсим Колонку B
        if (std::getline(line_stream, token, ',')) {
            row.column_b = token;
        }
        
        return row;
    }
};
