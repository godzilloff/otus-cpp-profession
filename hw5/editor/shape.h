#pragma once
#include <memory>
#include <string>

/**
 * @class Shape
 * @brief Abstract base class for all geometric shapes
 * 
 * Defines the common interface for all drawable shapes in the vector graphics editor.
 * Follows the Open-Closed Principle - new shapes can be added without modifying this class.
 */
class Shape {
public:
    /**
     * @brief Virtual destructor for polymorphic deletion
     * @ensures Proper cleanup of derived class resources
     */
    virtual ~Shape() = default;

    /**
     * @brief Pure virtual function for drawing the shape
     * @details Must be implemented by all concrete shape classes
     * @post The shape's visual representation should be rendered to output
     */
    virtual void draw() const = 0;

    /**
     * @brief Pure virtual function to get shape's identifier
     * @return Name of the shape as std::string
     * @details Each concrete shape must provide its naming logic
     */
    virtual std::string getName() const = 0;
};

/**
 * @typedef ShapePtr
 * @brief Shared smart pointer to Shape objects
 * 
 * Provides automatic memory management for Shape hierarchy.
 * Preferred over raw pointers for exception safety and ownership clarity.
 */
using ShapePtr = std::shared_ptr<Shape>;
