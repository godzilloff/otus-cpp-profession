/**
 * @file session.cpp
 * @brief Реализация класса Session для обработки клиентских соединений
 * @details Содержит обработку асинхронного сетевого взаимодействия с клиентами
 */

#include "session.h"
#include <iostream>

/**
 * @brief Конструктор сессии
 * @param socket Сокет для общения с клиентом
 * @param handler Обработчик команд для выполнения бизнес-логики
 * @details Перемещает переданные параметры в члены класса для управления ресурсами
 */
Session::Session(tcp::socket socket, std::unique_ptr<CommandHandler> handler) 
    : socket_(std::move(socket)), handler_(std::move(handler)) {}

/**
 * @brief Запускает сессию обработки клиентских команд
 * @details Начинает цикл обработки команд с чтения первой команды от клиента
 */
void Session::start() {
    read_command();
}

/**
 * @brief Асинхронно читает команду от клиента
 * @details Использует boost::asio для асинхронного чтения данных до символа новой строки.
 *          После получения команды передает ее на обработку в process_command()
 */
void Session::read_command() {
    auto self(shared_from_this());
    boost::asio::async_read_until(socket_, buffer_, '\n',
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                std::istream is(&buffer_);
                std::string line;
                std::getline(is, line);
                
                // Удаляем символ возврата каретки для Windows совместимости
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                
                process_command(line);
            } else {
                std::cout << "Client disconnected: " << ec.message() << std::endl;
            }
        });
}

/**
 * @brief Обрабатывает полученную команду от клиента
 * @param command Строка команды для обработки
 * @details Передает команду обработчику и отправляет результат обратно клиенту.
 *          В случае исключения отправляет клиенту сообщение об ошибке.
 */
void Session::process_command(const std::string& command) {
    try {
        // Обрабатываем команду через обработчик и получаем ответ
        std::string response = handler_->handle_command(command);
        send_response(response + "\n");
    } catch (const std::exception& e) {
        // В случае ошибки отправляем сообщение об ошибке клиенту
        send_response("ERR " + std::string(e.what()) + "\n");
    }
}

/**
 * @brief Асинхронно отправляет ответ клиенту
 * @param response Строка ответа для отправки
 * @details Создает копию ответа для безопасной асинхронной отправки.
 *          После успешной отправки продолжает чтение следующих команд.
 */
void Session::send_response(const std::string& response) {
    // Создаем shared_ptr копию ответа для безопасного использования в лямбде
    auto response_copy = std::make_shared<std::string>(response);
    
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(*response_copy),
        [this, self, response_copy](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                // Успешно отправили ответ, читаем следующую команду
                read_command();
            } else {
                std::cout << "Send response error: " << ec.message() << std::endl;
            }
        });
}
