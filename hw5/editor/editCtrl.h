#pragma once
#include <memory>
#include <iostream>
#include "document.h"

/**
 * @class EditorController
 * @ingroup Controller
 * @implements MVC-Controller
 * @brief Main controller class for the vector graphics editor (MVC pattern).
 *
 * Manages document operations including creation, import/export,
 * and shape manipulation. Uses a unique_ptr to own the current document.
 */
class EditorController {
public:
    /**
     * @brief Default constructor.
     * @post Creates a new empty Document automatically.
     */
    EditorController() : currentDocument(std::make_unique<Document>()) {};

    /**
     * @brief Imports document from file.
     * @param filename Path to the file to import.
     * @note Currently outputs mock implementation.
     */
    void importDocument(const std::string& filename) {
        std::cout << "importDocument: " << filename << std::endl;
    };

    /**
     * @brief Exports document to file.
     * @param filename Destination file path.
     * @note Currently outputs mock implementation.
     * @warning Marked as const but may modify file system.
     */
    void exportDocument(const std::string& filename) const {
        std::cout << "exportDocument: " << filename << std::endl;
    };

    /**
     * @brief Creates a new blank document.
     * @post Replaces current document with new instance.
     * @note Automatically handles destruction of previous document.
     */
    void createNewDocument() {
        currentDocument = std::make_unique<Document>();
        std::cout << "New document created" << std::endl;
    }
    /**
     * @brief Provides read-only access to the current document
     * @return Const reference to the managed Document
     * @throws std::runtime_error if no document exists
     */
    const Document& getDocument() const {
        if (!currentDocument) {
            throw std::runtime_error("No active document!");
        }
        return *currentDocument;
    }

    /**
     * @brief Adds a shape to current document.
     * @param shape Shared pointer to the shape to add.
     * @throws std::runtime_error if no document exists (debug builds only).
     * @warning Transfers ownership via std::move.
     */
    void addShape(ShapePtr shape) {
        if (!currentDocument) {
            std::cerr << "Error: No active document!" << std::endl;
            return;
        }
        std::cout << "EditorController addShape: " << shape->getName() << std::endl;
        currentDocument->addShape(std::move(shape));
    }

    /**
     * @brief Removes shape by name from current document.
     * @param name Unique identifier of the shape to remove.
     * @return void
     * @post All shapes with matching name will be removed.
     * @note Safe to call even if name doesn't exist.
     */
    void removeShape(const std::string& name) {
        if (!currentDocument) {
            std::cerr << "Error: No active document!" << std::endl;
            return;
        }
        std::cout << "Doc removeShape: " << name << std::endl;
        currentDocument->removeShape(name);
    }

private:
    std::unique_ptr<Document> currentDocument; ///< Active document owned exclusively
};
