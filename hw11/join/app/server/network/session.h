#pragma once
#include <boost/asio.hpp>
#include <memory>
#include "command_handler.h"

using boost::asio::ip::tcp;

/**
 * @class Session
 * @brief Класс для управления сессией клиентского соединения
 * 
 * Обрабатывает асинхронное взаимодействие с клиентом: чтение команд,
 * обработку и отправку ответов. Каждая сессия работает в своем собственном
 * контексте и не блокирует основной поток сервера.
 */
class Session : public std::enable_shared_from_this<Session> {
public:
    /**
     * @brief Конструктор сессии
     * @param socket Сокет для общения с клиентом
     * @param handler Обработчик команд для выполнения бизнес-логики
     */
    Session(tcp::socket socket, std::unique_ptr<CommandHandler> handler);
    
    /**
     * @brief Запускает сессию
     * 
     * Инициирует асинхронное чтение первой команды от клиента.
     * После вызова этого метода сессия начинает работать автономно.
     */
    void start();

private:
    /**
     * @brief Асинхронно читает команду от клиента
     * 
     * Читает данные из сокета до символа новой строки (\n).
     * После успешного чтения вызывает process_command().
     */
    void read_command();
    
    /**
     * @brief Обрабатывает полученную команду
     * @param command Текст команды от клиента
     * 
     * Передает команду обработчику и готовит ответ для отправки.
     * В случае ошибки формирует сообщение об ошибке.
     */
    void process_command(const std::string& command);
    
    /**
     * @brief Отправляет ответ клиенту
     * @param response Текст ответа для отправки
     * 
     * Асинхронно отправляет ответ клиенту. После успешной отправки
     * снова вызывает read_command() для чтения следующей команды.
     */
    void send_response(const std::string& response);

    tcp::socket socket_;                          ///< Сокет для связи с клиентом
    boost::asio::streambuf buffer_;               ///< Буфер для чтения данных из сокета
    std::unique_ptr<CommandHandler> handler_;     ///< Обработчик команд (бизнес-логика)
};
