#pragma once

#include "observer.h"
#include <iostream>

/**
 * @brief Наблюдатель для вывода в консоль
 */
class ConsoleObserver : public IObserver {
public:
    void update(const std::vector<std::string>& commands, 
                time_t timestamp, 
                void* context_id) override {
        (void)timestamp; // Не используется в консоли
        (void)context_id; // Не используется в консоли
        
        if (commands.empty()) return;
        
        std::cout << "bulk: ";
        for (size_t i = 0; i < commands.size(); ++i) {
            if (i != 0) std::cout << ", ";
            std::cout << commands[i];
        }
        std::cout << std::endl;
    }
};
