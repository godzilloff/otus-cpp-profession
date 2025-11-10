#pragma once
#include <boost/asio.hpp>
#include <memory>
#include "../database/database.h"
#include "../database/repository.h"
#include "session.h"

using boost::asio::ip::tcp;

/**
 * @class Server
 * @brief Основной класс сервера для обработки операций над множествами
 * 
 * Класс реализует асинхронный TCP-сервер, который принимает подключения клиентов
 * и обрабатывает команды для операций над множествами (пересечение, симметрическая разность).
 * Использует Boost.Asio для асинхронных сетевых операций.
 */
class Server {
public:
    /**
     * @brief Конструктор сервера
     * @param io_context Контекст ввода-вывода Boost.Asio
     * @param port Порт для прослушивания входящих подключений
     * 
     * Инициализирует acceptor для указанного порта и начинает асинхронное
     * ожидание подключений клиентов.
     */
    Server(boost::asio::io_context& io_context, short port);
    
private:
    /**
     * @brief Начинает асинхронное ожидание нового подключения
     * 
     * При успешном подключении создает новую сессию для обработки команд клиента.
     * После обработки подключения немедленно начинает ожидание следующего.
     */
    void do_accept();
    
    tcp::acceptor acceptor_;                          ///< Acceptor для принятия входящих подключений
    std::shared_ptr<Database> db_;                    ///< Объект базы данных для хранения таблиц
    std::shared_ptr<ITableRepository> repository_;    ///< Репозиторий для операций с таблицами
};
