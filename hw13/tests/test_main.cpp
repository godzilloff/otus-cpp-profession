/**
 * @file test_main.cpp
 * @brief Главный файл тестов
 */

#include <gtest/gtest.h>

/**
 * @brief Точка входа для тестов
 */
int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    std::cout << "Running Fashion MNIST tests..." << std::endl;
    return RUN_ALL_TESTS();
}
