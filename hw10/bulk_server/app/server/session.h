// app/server/session.h
#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <functional>
#include "async.h"

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, size_t bulk_size,
            std::function<void(std::shared_ptr<Session>)> remove_callback);
    ~Session();
    
    void start();

private:
    void do_read();
    void process_line(const std::string& line);
    
    tcp::socket socket_;
    boost::asio::streambuf buffer_;
    async_lib::handle_t processor_handle_;  // КАЖДАЯ СЕССИЯ ИМЕЕТ СВОЙ ОБРАБОТЧИК!
    std::function<void(std::shared_ptr<Session>)> remove_callback_;
    size_t bulk_size_;
};
