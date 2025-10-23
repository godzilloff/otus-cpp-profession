#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "async_server.h"

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: bulk_server <port> <bulk_size>\n";
            return 1;
        }
        
        int port_num = std::atoi(argv[1]);
        size_t bulk_size = std::atoi(argv[2]);
        
        if (port_num <= 0 || port_num > 65535 || bulk_size <= 0) {
            std::cerr << "Invalid arguments\n";
            return 1;
        }
        
        short port = static_cast<short>(port_num);
        
        boost::asio::io_context io_context;
        
        AsyncServer server(io_context, port, bulk_size);
        server.run();
        
        std::cout << "Bulk server is running. Press Ctrl+C to stop." << std::endl;
        
        io_context.run();
        
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
