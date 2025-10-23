/**
 * @file simple_client.cpp
 * @brief Простой TCP-клиент для тестирования bulk-сервера
 * 
 * Клиент подключается к серверу обработки bulk-команд и позволяет
 * в интерактивном режиме отправлять команды для тестирования функциональности
 * сервера. Поддерживает ручной ввод команд и передачу данных через pipe.
 */

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>

/**
 * @brief Класс простого TCP-клиента для тестирования сервера
 * 
 * Обеспечивает подключение к серверу, интерактивный ввод команд
 * и корректное завершение соединения. Использует синхронные операции
 * Boost.ASIO для простоты реализации.
 */
class SimpleClient {
public:
    /**
     * @brief Конструктор клиента
     * @param host Хост или IP-адрес сервера для подключения
     * @param port Порт сервера для подключения
     * 
     * @note Поддерживаются как доменные имена (localhost), так и IP-адреса (127.0.0.1)
     */
    SimpleClient(const std::string& host, const std::string& port)
        : host_(host), port_(port) {}
    
    /**
     * @brief Запускает клиентский цикл работы
     * @return true если работа завершена успешно, false в случае ошибки
     * 
     * Выполняет последовательно:
     * 1. Разрешение доменного имени и порта
     * 2. Подключение к серверу
     * 3. Интерактивный ввод команд от пользователя
     * 4. Корректное закрытие соединения
     * 
     * @warning Метод блокирующий - управление вернется только после завершения работы
     * 
     * @code
     * // Пример использования
     * SimpleClient client("localhost", "9000");
     * bool success = client.run();
     * @endcode
     */
    bool run() {
        try {
            // Контекст ввода-вывода Boost.ASIO
            boost::asio::io_context io_context;
            // TCP-сокет для общения с сервером
            boost::asio::ip::tcp::socket socket(io_context);
            // Резолвер для преобразования хоста и порта в endpoint'ы
            boost::asio::ip::tcp::resolver resolver(io_context);
            
            std::cout << "Connecting to " << host_ << ":" << port_ << "..." << std::endl;
            
            /**
             * @brief Разрешение хоста и порта в список endpoint'ов
             * 
             * Преобразует текстовые представления хоста и порта
             * в список сетевых endpoint'ов для подключения.
             */
            auto endpoints = resolver.resolve(host_, port_);
            
            /**
             * @brief Подключение к серверу
             * 
             * Выполняет попытку подключения ко всем endpoint'ам из списка
             * до первого успешного подключения.
             */
            boost::asio::connect(socket, endpoints);
            
            // Получаем информацию о подключении СРАЗУ после подключения
            std::string remote_endpoint_info = "unknown";
            try {
                auto remote_ep = socket.remote_endpoint();
                remote_endpoint_info = remote_ep.address().to_string() + ":" + std::to_string(remote_ep.port());
                std::cout << "Connected successfully to: " << remote_endpoint_info << std::endl;
            } catch (const boost::system::system_error& e) {
                std::cout << "Connected but could not get remote endpoint: " << e.what() << std::endl;
                return false;
            }
            
            /**
             * @brief Цикл интерактивного ввода команд
             * 
             * Читает команды из стандартного ввода и отправляет их на сервер.
             * Пустая строка или Ctrl+D завершают работу клиента.
             */
            std::string line;
            std::cout << "Enter commands (empty line to exit):" << std::endl;
            std::cout << "> ";
            
            while (std::getline(std::cin, line)) {
                // Пустая строка - сигнал завершения работы
                if (line.empty()) {
                    std::cout << "Empty line - exiting..." << std::endl;
                    break;
                }
                
                // Добавляем символ новой строки (сервер ожидает \n)
                std::string data = line + "\n";
                
                try {
                    // Проверяем, что сокет еще подключен
                    if (!socket.is_open()) {
                        std::cout << "Socket is closed. Cannot send command." << std::endl;
                        break;
                    }
                    
                    /**
                     * @brief Отправка данных на сервер
                     * 
                     * Синхронно записывает данные в сокет. Гарантирует,
                     * что все данные будут отправлены или возникнет исключение.
                     */
                    size_t bytes_written = boost::asio::write(socket, boost::asio::buffer(data));
                    std::cout << "Sent " << bytes_written << " bytes: " << line << std::endl;
                    
                } catch (const boost::system::system_error& e) {
                    std::cout << "Failed to send command: " << e.what() << std::endl;
                    break;
                }
                
                // Приглашение для следующей команды
                std::cout << "> ";
                
                // Небольшая задержка для имитации реального ввода
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            /**
             * @brief Корректное закрытие соединения
             * 
             * Выполняет graceful shutdown сокета для уведомления сервера
             * о завершении передачи данных, затем закрывает сокет.
             */
            try {
                if (socket.is_open()) {
                    boost::system::error_code ec;
                    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                    if (ec) {
                        std::cout << "Socket shutdown error: " << ec.message() << std::endl;
                    }
                    socket.close();
                }
            } catch (const boost::system::system_error& e) {
                std::cout << "Socket close error: " << e.what() << std::endl;
            }
            
            std::cout << "Connection closed" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Client error: " << e.what() << std::endl;
            return false;
        }
    }

private:
    std::string host_;   ///< Хост или IP-адрес сервера для подключения
    std::string port_;   ///< Порт сервера для подключения
};

/**
 * @brief Точка входа в программу клиента
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return int Код завершения программы (0 - успех, 1 - ошибка)
 * 
 * @par Использование:
 * @code{.sh}
 * // Интерактивный режим
 * simple_client localhost 9000
 * 
 * // Передача команд через pipe
 * echo -e "cmd1\ncmd2\ncmd3" | simple_client 127.0.0.1 9000
 * 
 * // Чтение команд из файла
 * cat commands.txt | simple_client localhost 9000
 * @endcode
 * 
 * @note Для работы клиента должен быть запущен bulk_server на указанном порту
 * 
 * @warning Программа не возвращает управление до завершения работы пользователем
 *          или разрыва соединения с сервером.
 */
int main(int argc, char* argv[]) {
    // Проверка корректности аргументов командной строки
    if (argc != 3) {
        std::cerr << "Usage: simple_client <host> <port>\n";
        std::cerr << "Example: simple_client 127.0.0.1 9000\n";
        return 1;
    }

    // Извлечение параметров подключения из аргументов
    std::string host = argv[1];
    std::string port = argv[2];

    // Создание и запуск клиента
    SimpleClient client(host, port);
    bool success = client.run();
    
    // Завершение работы с соответствующим кодом возврата
    std::cout << "Client finished" << std::endl;
    return success ? 0 : 1;
}
