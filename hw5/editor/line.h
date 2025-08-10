#pragma once
#include "shape.h"

/**
 * @class Line
 * @brief Concrete implementation of a line shape
 *
 * Inherits from Shape base class and implements all pure virtual methods.
 * Represents a straight line segment with configurable name and drawing capabilities.
 */
class Line : public Shape {
public:
    /**
     * @brief Default constructor
     * @post Creates a line with empty name
     * @note Outputs construction message to stdout for debugging
     * @warning Currently incorrectly outputs "ctor Circle" (should be "ctor Line")
     */
    Line() { std::cout << "ctor Circle" << std::endl; };

    /**
     * @brief Destructor
     * @note Outputs destruction message to stdout for debugging
     * @warning Currently incorrectly outputs "~dtor Circle" (should be "~dtor Line")
     */
    ~Line() { std::cout << "~dtor Circle" << std::endl; };

    /**
     * @brief Renders the line to output device
     * @implements Shape::draw()
     * @note Currently outputs mock drawing to stdout
     * @post Outputs "draw line" to standard output
     */
    void draw() const override { std::cout << "draw line" << std::endl; };

    /**
     * @brief Gets the name of the line
     * @return Current name of the line as std::string
     * @implements Shape::getName()
     */
    std::string getName() const override { return name; };

    /**
     * @brief Sets the name of the line
     * @param name_ New name to assign to the line
     * @post The line's internal name will be updated
     */
    void setName(const std::string& name_) { name = name_; };

private:
    std::string name; ///< Internal storage for the line's identifier
};
