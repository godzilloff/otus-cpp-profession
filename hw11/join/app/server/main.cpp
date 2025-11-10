/**
 * @file main.cpp
 * @brief Главный файл сервера операций над множествами
 * @details
 * Сервер обрабатывает команды для работы с двумя таблицами (A и B),
 * выполняет операции пересечения и симметрической разности над множествами.
 * 
 * Протокол команд:
 * - INSERT table id name    - вставка записи в таблицу
 * - TRUNCATE table          - очистка таблицы  
 * - INTERSECTION            - пересечение таблиц A и B
 * - SYMMETRIC_DIFFERENCE    - симметрическая разность таблиц A и B
 */

#include <iostream>
#include <memory>
#include "network/server.h"

using boost::asio::ip::tcp;

/**
 * @brief Точка входа серверного приложения
 * @param argc Количество аргументов командной строки
 * @param argv Аргументы командной строки
 * @return Код завершения программы
 * @retval 0 Успешное завершение
 * @retval 1 Ошибка в аргументах или работе сервера
 * 
 * @par Пример использования:
 * @code{.sh}
 * ./server 9000
 * @endcode
 * 
 * @note Для работы сервера требуется указать порт в качестве аргумента
 * @warning Сервер должен быть запущен с правами доступа к указанному порту
 * 
 * @par Структура таблиц:
 * Таблицы A и B имеют идентичную структуру:
 * - id (INTEGER PRIMARY KEY) - уникальный идентификатор
 * - name (TEXT NOT NULL)     - строковое значение
 * 
 * @see Server
 * @see Database
 * @see TableRepository
 */
int main(int argc, char* argv[]) {
    try {
        // Проверка корректности аргументов командной строки
        if (argc != 2) {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }

        // Создание контекста ввода-вывода для асинхронных операций
        boost::asio::io_context io_context;
        
        // Создание и запуск сервера на указанном порту
        Server server(io_context, static_cast<short>(std::atoi(argv[1])));
        
        // Запуск основного цикла обработки событий
        io_context.run();
        
    } catch (std::exception& e) {
        // Обработка исключений с выводом диагностической информации
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
