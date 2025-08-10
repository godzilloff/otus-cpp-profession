#include <iostream>
#include <string>

#include "editCtrl.h"
#include "view.h"
#include "circle.h"
#include "line.h"

/**
 * @brief Main program function
 * @details Demonstrates print_ip function with different data types
 * @return 0 on success
 */
int main() {
    EditorController editor;
    ConsoleView view;
    editor.createNewDocument();

    // add 5 circle
    for (int i = 0; i < 5; ++i) {
        auto circle = std::make_shared<Circle>();
        std::string str = "circle" + std::to_string(i);
        circle->setName(str);
        editor.addShape(circle);
    }

    view.displayDocument(editor.getDocument());
    
    // add 4 line
    for (int i = 0; i < 4; ++i) {
        auto line = std::make_shared<Line>();
        std::string str = "line" + std::to_string(i);
        line->setName(str);
        editor.addShape(line);
    }

    view.displayDocument(editor.getDocument());
    
    // remove 5 circle
    for (int i = 0; i < 5; ++i) {
        std::string str = "circle" + std::to_string(i);
        editor.removeShape(str);
    }

    // remove 4 line
    for (int i = 0; i < 4; ++i) {
        std::string str = "line" + std::to_string(i);
        editor.removeShape(str);
    }
    
    view.displayDocument(editor.getDocument());

    editor.exportDocument("output.svg");

    std::cout << "end" << std::endl;
    return 0;
}
