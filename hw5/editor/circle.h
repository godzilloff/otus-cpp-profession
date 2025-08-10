#pragma once
#include "shape.h"

/**
 * @class Circle
 * @brief Concrete implementation of a circle shape
 *
 * Inherits from Shape base class and implements all pure virtual methods.
 * Represents a circle with configurable name and drawing capabilities.
 */
class Circle : public Shape {
public:
    /**
     * @brief Default constructor
     * @post Creates a circle with empty name
     * @note Outputs construction message to stdout
     */
    Circle() { std::cout << "ctor Circle" << std::endl; };

    /**
     * @brief Destructor
     * @note Outputs destruction message to stdout
     */
    ~Circle() { std::cout << "~dtor Circle" << std::endl; };

    /**
     * @brief Draws the circle to output device
     * @implements Shape::draw()
     * @note Currently outputs mock drawing to stdout
     */
    void draw() const override { std::cout << "draw circle" << std::endl; };

    /**
     * @brief Gets the name of the circle
     * @return Current name of the circle
     * @implements Shape::getName()
     */
    std::string getName() const override { return name; };

    /**
     * @brief Sets the name of the circle
     * @param name_ New name to assign
     * @post The circle's name will be updated
     */
    void setName(const std::string& name_) { name = name_; };

private:
    std::string name; ///< Internal storage for circle's identifier
};