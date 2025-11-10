#pragma once
#include <boost/asio.hpp>
#include <string>
#include <memory>
#include <iostream>

using boost::asio::ip::tcp;

/**
 * @class NetworkClient
 * @brief Класс для сетевого взаимодействия с сервером операций над множествами
 * 
 * Обеспечивает подключение к серверу, отправку команд и получение ответов
 * с использованием Boost.Asio для асинхронных операций.
 */
class NetworkClient {
public:
    /**
     * @brief Конструктор клиента
     * @param host Хост или IP-адрес сервера
     * @param port Порт сервера
     */
    NetworkClient(const std::string& host, const std::string& port);
    
    /**
     * @brief Деструктор клиента
     * @details Автоматически закрывает соединение при уничтожении объекта
     */
    ~NetworkClient();
    
    /**
     * @brief Установка соединения с сервером
     * @return true если соединение установлено успешно, false в случае ошибки
     */
    bool connect();
    
    /**
     * @brief Закрытие соединения с сервером
     * @details Безопасно завершает сетевое соединение и освобождает ресурсы
     */
    void disconnect();
    
    /**
     * @brief Проверка состояния соединения
     * @return true если соединение активно, false если соединение разорвано
     */
    bool is_connected() const;
    
    /**
     * @brief Отправка команды на сервер
     * @param command Текст команды для отправки (без завершающего перевода строки)
     * @return Ответ сервера в виде строки
     * @throw std::runtime_error в случае ошибки сети или отсутствия соединения
     * 
     * @note Команда автоматически дополняется символом новой строки '\n'
     * @see Протокол команд: INSERT, TRUNCATE, INTERSECTION, SYMMETRIC_DIFFERENCE
     */
    std::string send_command(const std::string& command);
    
    /**
     * @brief Гарантирует наличие активного соединения
     * @details При необходимости разрывает текущее и устанавливает новое соединение
     * @throw std::runtime_error если не удалось установить соединение
     */
    void ensure_connection();

private:
    /**
     * @brief Чтение ответа от сервера
     * @return Накопленный ответ сервера
     * @throw std::runtime_error в случае ошибки чтения
     * 
     * @details Читает данные от сервера построчно до получения маркера завершения (OK/ERR)
     * Автоматически обрабатывает разрыв соединения после выполнения команды
     */
    std::string read_response();
    
    std::string host_;                         ///< Хост сервера для подключения
    std::string port_;                         ///< Порт сервера для подключения
    boost::asio::io_context io_context_;       ///< Контекст ввода-вывода Boost.Asio
    std::unique_ptr<tcp::socket> socket_;      ///< Сокет для сетевого взаимодействия
    bool connected_;                           ///< Флаг состояния соединения
};
