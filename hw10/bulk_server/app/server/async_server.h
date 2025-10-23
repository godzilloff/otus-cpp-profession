// app/server/async_server.h
#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <unordered_set>
#include <mutex>
#include "async.h"

using boost::asio::ip::tcp;

class Session; // Предварительное объявление

class AsyncServer {
public:
    AsyncServer(boost::asio::io_context& io_context, short port, size_t bulk_size);
    ~AsyncServer();
    
    void run();
    void stop();
    void remove_session(std::shared_ptr<Session> session);
    size_t get_bulk_size() const { return bulk_size_; }

private:
    void do_accept();
    
    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    size_t bulk_size_;
    std::unordered_set<std::shared_ptr<Session>> sessions_;
    std::mutex sessions_mutex_;
};
