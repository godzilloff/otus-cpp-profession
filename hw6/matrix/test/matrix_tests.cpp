#include <gtest/gtest.h>
#include <iostream>

#include "matrix.h"

const size_t CNT = 10;

struct MatrixFixture : public testing::Test {
    const size_t cnt = CNT;
    Matrix<int,0> matrix;

    // Per-test-suite set-up.
    static void SetUpTestSuite() {
        // std::cout << "SetUpTestSuite" << std::endl;
    }

    // Per-test-suite tear-down.
    static void TearDownTestSuite() {
        // std::cout << "TearDownTestSuite" << std::endl;
    }

    // Per-test set-up
    void SetUp() override {
        std::cout << "SetUp" << std::endl;

        // Create list with elementCount elements    // Заполнение главной диагонали
        for (size_t i = 0; i < CNT; ++i) {
            matrix[i][i] = i;
        }

        // Заполнение второстепенной диагонали
        for (size_t i = 0; i < CNT; ++i) {
            matrix[i][9 - i] = 9 - i;
        }

    }

    // You can define per-test tear-down logic as usual.
    void TearDown() override {
        // std::cout << "TearDown" << std::endl;
    }
};


TEST(Matrix, Empty) {
    // Arrange
    Matrix<int,0> matrix_empty;

    // Act (empty for this test)

    // Assert
    ASSERT_EQ(matrix_empty.size(), static_cast<size_t>(0));
    ASSERT_TRUE(matrix_empty.empty());
}

TEST(Matrix, PushBack) {
    // Arrange
    const size_t count = CNT;
    Matrix<int, 0> matrix;

    // Act
    for (size_t i = 0; i < count; ++i)
        matrix[i][i] = i;

    // Заполнение второстепенной диагонали
    for (size_t i = 0; i < count; ++i)
        matrix[i][9 - i] = 9 - i;

    // Assert
    ASSERT_FALSE(matrix.empty());
    ASSERT_EQ(matrix.size(), static_cast<size_t>(18));
}


TEST_F(MatrixFixture, Operator) {
    // Arrange
    // from fixture

    // Act    
    ((matrix[100][100] = 314) = 0) = 217;
    std::cout << "matrix[100][100] = " << matrix[100][100] << std::endl;

    // Assert
    ASSERT_EQ(matrix.size(), static_cast<size_t>(19));
    ASSERT_FALSE(matrix.empty());

    ASSERT_EQ(matrix[100][100], static_cast<size_t>(217));
}
