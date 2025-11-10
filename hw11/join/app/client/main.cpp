/**
 * @file simple_client.cpp
 * @brief Клиент для тестирования сервера операций над множествами
 * @details 
 * Клиент подключается к серверу, заполняет таблицы A и B тестовыми данными
 * и выполняет операции пересечения и симметрической разности.
 * 
 * Протокол взаимодействия:
 * - INSERT table id name - добавление записи
 * - TRUNCATE table - очистка таблицы  
 * - INTERSECTION - операция пересечения
 * - SYMMETRIC_DIFFERENCE - операция симметрической разности
 */

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

using boost::asio::ip::tcp;

/**
 * @brief Отправляет одиночную команду серверу и выводит ответ
 * @param host Хост сервера
 * @param port Порт сервера
 * @param command Команда для отправки (без завершающего перевода строки)
 * 
 * @details
 * Функция создает новое соединение для каждой команды, отправляет команду
 * серверу и читает многострочный ответ до получения OK или ERR.
 * После выполнения команды соединение закрывается.
 * 
 * Пример команды: "INSERT A 0 lean"
 * Пример ответа: "OK" или "ERR duplicate 0"
 */
void send_single_command(const std::string& host, const std::string& port, 
                        const std::string& command) {
    try {
        std::cout << "Sending: " << command << std::endl;
        
        // Создание контекста ввода-вывода и сокета
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        tcp::resolver resolver(io_context);
        
        // Разрешение доменного имени и установка соединения
        auto endpoints = resolver.resolve(host, port);
        boost::asio::connect(socket, endpoints);
        
        // Отправка команды с добавлением символа новой строки
        std::string full_command = command + "\n";
        boost::asio::write(socket, boost::asio::buffer(full_command));
        
        // Чтение ответа от сервера
        boost::asio::streambuf buffer;
        std::string response;
        
        while (true) {
            boost::system::error_code ec;
            // Чтение до символа новой строки
            boost::asio::read_until(socket, buffer, '\n', ec);
            
            // Обработка ошибок (кроме нормального завершения EOF)
            if (ec && ec != boost::asio::error::eof) {
                throw boost::system::system_error(ec);
            }
            
            // Извлечение строки из буфера
            std::istream is(&buffer);
            std::string line;
            std::getline(is, line);
            
            // Удаление символа возврата каретки (для Windows совместимости)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            std::cout << "Received: " << line << std::endl;
            response += line + "\n";
            
            // Завершение чтения при получении OK, ERR или конца файла
            if (line == "OK" || line.find("ERR") == 0 || ec == boost::asio::error::eof) {
                break;
            }
        }
        
        // Закрытие соединения и небольшая задержка
        socket.close();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Главная функция клиента
 * @param argc Количество аргументов командной строки
 * @param argv Аргументы командной строки
 * @return 0 при успешном выполнении, 1 при ошибке
 * 
 * @details
 * Ожидаемые аргументы:
 * - argv[1]: хост сервера (например, "localhost")
 * - argv[2]: порт сервера (например, "9000")
 * 
 * Последовательность выполнения:
 * 1. Инициализация таблицы A тестовыми данными
 * 2. Инициализация таблицы B тестовыми данными  
 * 3. Выполнение операции пересечения (INTERSECTION)
 * 4. Выполнение операции симметрической разности (SYMMETRIC_DIFFERENCE)
 * 
 * Тестовые данные соответствуют условию задачи:
 * - Таблица A: {0: "lean", 1: "sweater", 2: "frank", 3: "violation", 4: "quality", 5: "precision"}
 * - Таблица B: {3: "proposal", 4: "example", 5: "lake", 6: "flour", 7: "wonder", 8: "selection"}
 */
int main(int argc, char* argv[]) {
    // Проверка аргументов командной строки
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        std::cerr << "Example: " << argv[0] << " localhost 9000" << std::endl;
        return 1;
    }
    
    const std::string host = argv[1];
    const std::string port = argv[2];
    
    /// Инициализация таблицы A тестовыми данными
    /// @see Структура таблицы A из условия задачи
    send_single_command(host, port, "INSERT A 0 lean");
    send_single_command(host, port, "INSERT A 1 sweater");
    send_single_command(host, port, "INSERT A 2 frank");
    send_single_command(host, port, "INSERT A 3 violation");
    send_single_command(host, port, "INSERT A 4 quality");
    send_single_command(host, port, "INSERT A 5 precision");
    
    /// Инициализация таблицы B тестовыми данными
    /// @see Структура таблицы B из условия задачи  
    send_single_command(host, port, "INSERT B 3 proposal");
    send_single_command(host, port, "INSERT B 4 example");
    send_single_command(host, port, "INSERT B 5 lake");
    send_single_command(host, port, "INSERT B 6 flour");
    send_single_command(host, port, "INSERT B 7 wonder");
    send_single_command(host, port, "INSERT B 8 selection");
    
    /// Выполнение операций над множествами
    /// @note INTERSECTION - пересечение множеств A и B
    /// @note SYMMETRIC_DIFFERENCE - симметрическая разность множеств A и B
    send_single_command(host, port, "INTERSECTION");
    send_single_command(host, port, "SYMMETRIC_DIFFERENCE");
    
    return 0;
}
