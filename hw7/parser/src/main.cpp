#include <iostream>
#include <string>
#include <cassert>
#include "parse.h"

/**
 * @brief Main program function
 * @details Demonstrates parser function
 * @return 0 on success
 */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <bulk_size>" << std::endl;
        return 1;
    }
    
    size_t bulk_size = std::stoul(argv[1]);
    
    auto observable = std::make_shared<Observable>();
    observable->subscribe(std::make_shared<ConsoleObserver>());
    observable->subscribe(std::make_shared<FileObserver>());
    
    CommandProcessor processor(bulk_size);
    processor.set_observable(observable);
    
    std::string line;
    while (std::getline(std::cin, line)) {
        processor.process_command(line);
    }
    
    processor.flush_remaining();
    
    return 0;
}
