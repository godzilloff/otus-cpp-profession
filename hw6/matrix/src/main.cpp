#include <iostream>
#include <string>
#include <cassert>
#include "matrix.h"

/**
 * @brief Main program function
 * @details Demonstrates matrix function
 * @return 0 on success
 */
int main() {
    Matrix<int, 0> matrix;

    // Заполнение главной диагонали
    for (size_t i = 0; i < 10; ++i) {
        matrix[i][i] = i;
    }

    // Заполнение второстепенной диагонали
    for (size_t i = 0; i < 10; ++i) {
        matrix[i][9 - i] = 9 - i;
    }

    // Вывод фрагмента матрицы
    std::cout << "Matrix fragment [1,1] to [8,8]:" << std::endl;
    matrix.printFragment(1, 1, 8, 8);

    // Вывод количества занятых ячеек
    std::cout << "\nOccupied cells count: " << matrix.size() << std::endl;

    // Вывод всех занятых ячеек
    std::cout << "\nAll occupied cells:" << std::endl;
    for (auto [indices, value] : matrix) {
        auto [x, y] = indices;
        std::cout << "[" << x << "][" << y << "] = " << value << std::endl;
    }

    ((matrix[100][100] = 314) = 0) = 217;
    std::cout << "matrix[100][100] = " << matrix[100][100] << std::endl;

    return 0;
}
