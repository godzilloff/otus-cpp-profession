#pragma once
#include <iostream>
#include "document.h"

/**
 * @class ConsoleView
 * @ingroup View
 * @implements MVC-View
 * @brief Console-based view following MVC pattern (replaces GUI in this implementation)
 */
class ConsoleView {
public:
    /**
     * @brief Renders document state to console
     * @param doc Reference to document model
     * @post Outputs:
     *       - Document size
     *       - All shapes' names
     */
    void displayDocument(const Document& doc) const {
        const auto& shapes = doc.getShapes();
        
        std::cout << "\n=== Document Contents ===" << std::endl;
        std::cout << "Total shapes: " << shapes.size() << std::endl;
        
        for (const auto& shape : shapes) {
            if (shape) { // check nullptr
                std::cout << "- " << shape->getName() << std::endl;
            }
        }
        std::cout << "=======================\n" << std::endl;
    }
};
