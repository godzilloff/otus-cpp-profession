/**
 * @file client.cpp
 * @brief Реализация сетевого клиента для взаимодействия с сервером операций над множествами
 * @details 
 * Этот модуль предоставляет функциональность для установки соединения с сервером,
 * отправки команд и получения ответов по протоколу операций над множествами.
 */

#include "client.h"
#include <iostream>
#include <stdexcept>

/**
 * @brief Конструктор сетевого клиента
 * @param host Хост сервера для подключения
 * @param port Порт сервера для подключения
 * @details
 * Инициализирует клиента с указанными параметрами подключения.
 * Соединение устанавливается при первом вызове connect().
 */
NetworkClient::NetworkClient(const std::string& host, const std::string& port)
    : host_(host), port_(port), socket_(nullptr), connected_(false) {}

/**
 * @brief Деструктор клиента
 * @details
 * Автоматически разрывает соединение при уничтожении объекта.
 * Гарантирует корректное освобождение ресурсов сокета.
 */
NetworkClient::~NetworkClient() {
    disconnect();
}

/**
 * @brief Установка соединения с сервером
 * @return true если соединение установлено успешно, false в случае ошибки
 * @details
 * Выполняет многоуровневую проверку соединения:
 * - Разрешение DNS имени хоста
 * - Установка TCP соединения
 * - Проверка открытого состояния сокета
 * - Тест активности соединения через неблокирующий прием
 * 
 * @throws std::exception в случае критических ошибок инициализации
 */
bool NetworkClient::connect() {
    try {
        // Проверяем, не установлено ли уже соединение
        if (connected_ && socket_ && socket_->is_open()) {
            return true;
        }
        
        // Создаем новый сокет и резолвер
        socket_ = std::make_unique<tcp::socket>(io_context_);
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, port_);
        
        // Устанавливаем соединение с явной проверкой ошибок
        boost::system::error_code ec;
        boost::asio::connect(*socket_, endpoints, ec);
        
        if (ec) {
            std::cerr << "Connection failed: " << ec.message() << std::endl;
            connected_ = false;
            socket_.reset();
            return false;
        }
        
        // Дополнительная проверка, что сокет действительно открыт
        if (!socket_->is_open()) {
            std::cerr << "Socket is not open after connect" << std::endl;
            connected_ = false;
            socket_.reset();
            return false;
        }
        
        /**
         * @brief Тестирование активности соединения
         * @details
         * Временно переводим сокет в неблокирующий режим и пытаемся прочитать 1 байт
         * в режиме peek (без извлечения из буфера). Это позволяет проверить, что
         * соединение действительно установлено и активно.
         */
        socket_->non_blocking(true);
        char test_buffer[1];
        boost::system::error_code test_ec;
        socket_->receive(boost::asio::buffer(test_buffer), 
                        tcp::socket::message_peek, test_ec);
        
        // Ошибка, отличная от would_block, указывает на проблему с соединением
        if (test_ec && test_ec != boost::asio::error::would_block) {
            std::cerr << "Connection test failed: " << test_ec.message() << std::endl;
            connected_ = false;
            socket_.reset();
            return false;
        }
        
        // Возвращаем сокет в блокирующий режим
        socket_->non_blocking(false);
        connected_ = true;
        std::cout << "Connected to " << host_ << ":" << port_ << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Connection exception: " << e.what() << std::endl;
        connected_ = false;
        socket_.reset();
        return false;
    }
}

/**
 * @brief Разрыв соединения с сервером
 * @details
 * Выполняет корректное закрытие соединения:
 * - Отправка сигнала shutdown для уведомления сервера
 * - Закрытие сокета
 * - Сброс состояния подключения
 * 
 * Метод устойчив к ошибкам и не бросает исключения.
 */
void NetworkClient::disconnect() {
    if (socket_ && socket_->is_open()) {
        try {
            boost::system::error_code ec;
            // Уведомляем сервер о разрыве соединения
            socket_->shutdown(tcp::socket::shutdown_both, ec);
            if (ec) {
                std::cout << "Socket shutdown warning: " << ec.message() << std::endl;
            }
            
            // Закрываем сокет
            socket_->close(ec);
            if (ec) {
                std::cout << "Socket close warning: " << ec.message() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Disconnect exception: " << e.what() << std::endl;
        }
    }
    connected_ = false;
    socket_.reset();
}

/**
 * @brief Проверка состояния соединения
 * @return true если соединение активно и сокет открыт, иначе false
 * @details
 * Выполняет комплексную проверку:
 * - Флаг connected_ установлен в true
 * - Указатель socket_ валиден
 * - Сокет находится в открытом состоянии
 */
bool NetworkClient::is_connected() const {
    return connected_ && socket_ && socket_->is_open();
}

/**
 * @brief Гарантирует установленное соединение
 * @throws std::runtime_error если не удалось установить соединение
 * @details
 * Если соединение не активно, выполняет переподключение.
 * Используется перед отправкой команд для обеспечения работоспособности.
 */
void NetworkClient::ensure_connection() {
    if (!is_connected()) {
        disconnect();
        if (!connect()) {
            throw std::runtime_error("Failed to establish connection");
        }
    }
}

/**
 * @brief Отправка команды серверу и получение ответа
 * @param command Текст команды для отправки (без символа новой строки)
 * @return Ответ сервера в виде строки
 * @throws std::runtime_error при ошибках сети или отправки данных
 * @details
 * Протокол взаимодействия:
 * 1. Команда дополняется символом новой строки
 * 2. Данные отправляются через сокет
 * 3. Читается ответ до получения маркера окончания (OK/ERR)
 * 
 * Поддерживаемые команды:
 * - INSERT table id name
 * - TRUNCATE table  
 * - INTERSECTION
 * - SYMMETRIC_DIFFERENCE
 */
std::string NetworkClient::send_command(const std::string& command) {
    // Гарантируем активное соединение
    ensure_connection();
    
    if (!is_connected()) {
        throw std::runtime_error("Not connected to server");
    }
    
    try {
        // Формируем полную команду с символом окончания
        std::string full_command = command + "\n";
        std::cout << "Sending: " << command << std::endl;
        
        // Отправляем команду с явной проверкой ошибок
        boost::system::error_code write_ec;
        std::size_t bytes_written = boost::asio::write(*socket_, 
            boost::asio::buffer(full_command), write_ec);
            
        if (write_ec) {
            std::cerr << "Write error: " << write_ec.message() << std::endl;
            connected_ = false;
            throw std::runtime_error("Write failed: " + write_ec.message());
        }
        
        // Проверяем, что отправлены все данные
        if (bytes_written != full_command.size()) {
            std::cerr << "Incomplete write: " << bytes_written 
                      << " of " << full_command.size() << " bytes" << std::endl;
            connected_ = false;
            throw std::runtime_error("Incomplete write");
        }
        
        std::cout << "Successfully sent " << bytes_written << " bytes" << std::endl;
        
        // Читаем ответ сервера
        return read_response();
        
    } catch (const std::exception& e) {
        connected_ = false;
        throw std::runtime_error("Send command failed: " + std::string(e.what()));
    }
}

/**
 * @brief Чтение ответа от сервера
 * @return Накопленный ответ сервера
 * @throws std::runtime_error при ошибках чтения или разрыве соединения
 * @details
 * Алгоритм чтения:
 * - Чтение построчно до символа новой строки
 * - Накопление строк в буфере ответа
 * - Остановка при получении OK или ERR
 * - Обработка нормального закрытия соединения (EOF)
 * 
 * Форматы ответов:
 * - OK - успешное выполнение команды
 * - ERR message - ошибка с описанием
 * - Многострочные ответы для операций над множествами
 */
std::string NetworkClient::read_response() {
    if (!is_connected()) {
        throw std::runtime_error("Socket not connected for reading");
    }
    
    boost::asio::streambuf buffer;
    std::string response;
    boost::system::error_code ec;
    
    // Читаем построчно до получения маркера окончания
    while (true) {
        std::size_t bytes_read = boost::asio::read_until(*socket_, buffer, '\n', ec);
        
        /**
         * @brief Обработка условий завершения чтения
         * @details
         * - EOF: сервер закрыл соединение после отправки ответа (нормальная ситуация)
         * - Другие ошибки: проблемы с сетью или соединением
         * - bytes_read > 0: есть данные для обработки даже при EOF
         */
        if (ec && (bytes_read > 0)) {
            if (ec == boost::asio::error::eof) {
                // Сервер закрыл соединение после обработки команды
                std::cout << "Connection closed by server (normal)" << std::endl;
                break;
            } else {
                std::cerr << "Read error: " << ec.message() << std::endl;
                connected_ = false;
                throw std::runtime_error("Read error: " + ec.message());
            }
        }
        
        // Извлекаем строку из буфера
        std::istream is(&buffer);
        std::string line;
        std::getline(is, line);
        
        // Удаляем символ возврата каретки (для Windows совместимости)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        std::cout << "Received: " << line << std::endl;
        response += line + "\n";
        
        // Завершаем чтение при получении финального маркера
        if (line == "OK" || line.find("ERR") == 0) {
            break;
        }
    }
    
    return response;
}
