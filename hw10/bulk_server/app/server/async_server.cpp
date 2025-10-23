// app/server/async_server.cpp
#include "async_server.h"
#include "session.h"
#include <iostream>

AsyncServer::AsyncServer(boost::asio::io_context& io_context, short port, size_t bulk_size)
    : io_context_(io_context)
    , acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    , bulk_size_(bulk_size) {
    
    std::cout << "Server started on port " << port 
              << " with bulk size " << bulk_size << std::endl;
}

AsyncServer::~AsyncServer() {
    stop();
}

void AsyncServer::run() {
    do_accept();
}

void AsyncServer::stop() {
    acceptor_.close();
    
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    // При остановке сервера завершаем все сессии
    sessions_.clear();
}

void AsyncServer::do_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::cout << "New client connected" << std::endl;
                
                // Создаем callback для удаления сессии
                auto remove_callback = [this](std::shared_ptr<Session> session) {
                    remove_session(session);
                };
                
                // Каждое соединение получает свою сессию
                auto session = std::make_shared<Session>(
                    std::move(socket), bulk_size_, remove_callback);
                
                {
                    std::lock_guard<std::mutex> lock(sessions_mutex_);
                    sessions_.insert(session);
                }
                
                session->start();
            } else {
                if (ec != boost::asio::error::operation_aborted) {
                    std::cerr << "Accept error: " << ec.message() << std::endl;
                }
            }
            
            // Продолжаем принимать новые соединения
            if (acceptor_.is_open()) {
                do_accept();
            }
        });
}

void AsyncServer::remove_session(std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.erase(session);
    std::cout << "Client disconnected. Active connections: " << sessions_.size() << std::endl;
}
