/**
 * @file main.cpp
 * @brief Демонстрационная программа для библиотеки асинхронной обработки команд
 * 
 * Программа демонстрирует использование всех функций библиотеки async:
 * - connect() для создания контекстов обработки
 * - receive() для передачи команд на обработку  
 * - disconnect() для корректного завершения работы
 * 
 * @note Программа создает несколько контекстов обработки и передает команды
 *       различного формата для тестирования всех возможностей библиотеки.
 */

#include "../include/async.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

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
    
    std::size_t bulk_size = std::stoul(argv[1]);

    auto h = async::connect(bulk_size);   ///< контекст обработки с размером блока bulk_size

    std::string line;
    while (std::getline(std::cin, line)) {
        async::receive(h, line.c_str(), line.size()); 
    }
        
    // Корректное завершение работы с контекстами
    async::disconnect(h);   ///< Освобождение ресурсов контекста
    
    return 0;
}
