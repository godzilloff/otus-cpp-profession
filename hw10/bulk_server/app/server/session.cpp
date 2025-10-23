// app/server/session.cpp
#include "session.h"
#include <iostream>

Session::Session(tcp::socket socket, size_t bulk_size,
                 std::function<void(std::shared_ptr<Session>)> remove_callback)
    : socket_(std::move(socket))
    , remove_callback_(remove_callback)
    , bulk_size_(bulk_size) {
    
    // КАЖДАЯ СЕССИЯ СОЗДАЕТ СВОЙ СОБСТВЕННЫЙ ОБРАБОТЧИК!
    processor_handle_ = async_lib::async_connect(bulk_size);
    std::cout << "Created new command processor for session" << std::endl;
}

Session::~Session() {
    // Завершаем обработчик КОНКРЕТНОЙ сессии
    async_lib::async_disconnect(processor_handle_);
    std::cout << "Destroyed command processor for session" << std::endl;
}

void Session::start() {
    do_read();
}

void Session::do_read() {
    auto self(shared_from_this());
    
    boost::asio::async_read_until(socket_, buffer_, '\n',
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec && (length > 0)) {
                std::istream is(&buffer_);
                std::string line;
                
                // Обрабатываем все строки из буфера
                while (std::getline(is, line)) {
                    // Убираем символ возврата каретки для Windows
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }
                    
                    if (!line.empty()) {
                        process_line(line);
                    }
                }
                
                // Продолжаем чтение
                do_read();
            } else {
                // При разрыве соединения уведомляем сервер
                if (ec == boost::asio::error::eof || 
                    ec == boost::asio::error::connection_reset) {
                    // Это нормальное завершение соединения
                    std::cout << "Client disconnected normally" << std::endl;
                } else {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                }
                
                if (remove_callback_) {
                    remove_callback_(shared_from_this());
                }
            }
        });
}

void Session::process_line(const std::string& line) {
    // Передаем команду в СОБСТВЕННЫЙ обработчик сессии
    async_lib::async_receive(processor_handle_, line.c_str(), line.size());
}
