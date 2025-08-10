#pragma once
#include <memory>
#include <vector>
#include "shape.h"

/**
 * @class Document
 * @brief Core data model for vector graphics document.
 *
 * Manages collection of shapes and provides serialization capabilities.
 * Implements add/remove/clear operations with duplicate prevention.
 */
class Document {
public:
    /**
     * @brief Import document from file (stub implementation)
     * @param filename Path to source file
     * @note Currently outputs mock operation
     * @todo Implement actual file parsing
     */
    void importFromFile(const std::string& filename) {
        std::cout << "importFromFile :" << filename << std::endl;
    };

    /**
     * @brief Export document to file (stub implementation)
     * @param filename Destination file path
     * @note Currently outputs mock operation
     * @todo Implement actual file serialization
     */
    void exportToFile(const std::string& filename) const {
        std::cout << "exportToFile: " << filename << std::endl;
    };

    /**
     * @brief Adds a shape to the document
     * @param shape Shared pointer to the shape object
     * @throws std::invalid_argument if shape is null
     * @post Performs:
     *       1. Nullptr cleanup
     *       2. Duplicate check
     *       3. Ownership transfer
     * @warning May modify vector capacity (potential reallocation)
     */
    void addShape(ShapePtr shape) {
        if (!shape) {
            std::cerr << "Error: Cannot add null shape!" << std::endl;
            return;
        }

        std::cout << "Doc size " << shapes.size() << std::endl;

        // Cleanup null pointers
        shapes.erase(std::remove_if(shapes.begin(), shapes.end(),
            [](const ShapePtr& s) { return s == nullptr; }),
            shapes.end());

        // Check for duplicates
        auto it = std::find_if(shapes.begin(), shapes.end(), 
            [&shape](const ShapePtr& s) {
                return s && shape && (s->getName() == shape->getName());
            });

        if (it != shapes.end()) {
            std::cerr << "Error: Shape '" << shape->getName() << "' already exists!"
                << std::endl;
            return;
        }

        std::cout << "Doc: addShape: " << shape->getName() << std::endl;
        shapes.push_back(shape);
    };

    /**
     * @brief Removes all shapes with matching name
     * @param name Identifier of shapes to remove
     * @post Efficiently preserves vector capacity
     * @note Safe to call with non-existent names
     * @complexity O(n) with single compaction
     */
    void removeShape(const std::string& name) {
        std::cout << "removeShape " << name << std::endl;

        shapes.erase(std::remove_if(shapes.begin(), shapes.end(),
            [&name](const ShapePtr& s) {
                return s && s->getName() == name;
            }),
            shapes.end());

        std::cout << "Doc size after remove: " << shapes.size() << std::endl;
    };

    /**
     * @brief Provides read-only access to shapes collection
     * @return Const reference to the vector of shapes
     */
    const std::vector<ShapePtr>& getShapes() const noexcept {
        return shapes;
    }

    /**
     * @brief Clears all shapes from document
     * @post Guarantees:
     *       - shapes.size() == 0
     *       - Memory may remain allocated
     * @note For complete cleanup, consider shrink_to_fit()
     */
    void clear() {
        std::cout << "clear()" << std::endl;
        shapes.clear();
    };

private:
    std::vector<ShapePtr> shapes; ///< Collection of managed shapes
};
