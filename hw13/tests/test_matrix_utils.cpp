/**
 * @file test_matrix_utils.cpp
 * @brief Тесты для MatrixUtils
 */

#include <gtest/gtest.h>
#include "MatrixUtils.h"

class MatrixUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_vector = {1.0f, 2.0f, 3.0f};
        test_matrix = {
            {1.0f, 2.0f, 3.0f},
            {4.0f, 5.0f, 6.0f}
        };
    }

    std::vector<float> test_vector;
    std::vector<std::vector<float>> test_matrix;
};

TEST_F(MatrixUtilsTest, SigmoidFunction) {
    auto result = MatrixUtils::sigmoid(test_vector);
    
    EXPECT_EQ(result.size(), test_vector.size());
    for (size_t i = 0; i < result.size(); i++) {
        EXPECT_GE(result[i], 0.0f);
        EXPECT_LE(result[i], 1.0f);
    }
}

TEST_F(MatrixUtilsTest, SoftmaxFunction) {
    auto result = MatrixUtils::softmax(test_vector);
    
    EXPECT_EQ(result.size(), test_vector.size());
    
    float sum = 0.0f;
    for (const auto& val : result) {
        sum += val;
        EXPECT_GE(val, 0.0f);
        EXPECT_LE(val, 1.0f);
    }
    
    EXPECT_NEAR(sum, 1.0f, 1e-6f);
}

TEST_F(MatrixUtilsTest, MatrixVectorMultiply) {
    std::vector<float> vector = {1.0f, 2.0f, 3.0f};
    auto result = MatrixUtils::matrixVectorMultiply(test_matrix, vector);
    
    EXPECT_EQ(result.size(), 2);
    EXPECT_FLOAT_EQ(result[0], 1*1 + 2*2 + 3*3); // 14
    EXPECT_FLOAT_EQ(result[1], 4*1 + 5*2 + 6*3); // 32
}

TEST_F(MatrixUtilsTest, MatrixVectorMultiplyInvalidDimensions) {
    std::vector<float> invalid_vector = {1.0f, 2.0f}; // Неправильный размер
    
    EXPECT_THROW({
        MatrixUtils::matrixVectorMultiply(test_matrix, invalid_vector);
    }, std::invalid_argument);
}

TEST_F(MatrixUtilsTest, Argmax) {
    std::vector<float> values = {0.1f, 0.9f, 0.2f, 0.3f};
    size_t max_index = MatrixUtils::argmax(values);
    
    EXPECT_EQ(max_index, 1);
}

TEST_F(MatrixUtilsTest, NormalizeImage) {
    std::vector<float> image = {0.0f, 128.0f, 255.0f};
    auto normalized = MatrixUtils::normalizeImage(image);
    
    EXPECT_FLOAT_EQ(normalized[0], 0.0f);
    EXPECT_FLOAT_EQ(normalized[1], 128.0f / 255.0f);
    EXPECT_FLOAT_EQ(normalized[2], 1.0f);
}
