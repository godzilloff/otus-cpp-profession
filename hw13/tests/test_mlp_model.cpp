/**
 * @file test_mlp_model.cpp
 * @brief Тесты для MLPModel
 */

#include <gtest/gtest.h>
#include "MLPModel.h"

class MLPModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Тестовые веса (маленькие)
        test_model_paths = {
            "test_data/sample_weights_w1.txt",
            "test_data/sample_weights_w2.txt"
        };
        
        // Основные веса (полные)
        main_model_paths = {
            "test_data/w1.txt",
            "test_data/w2.txt"
        };
        
        test_image = std::vector<float>(784, 0.0f);
        test_image_with_values = std::vector<float>(784, 0.0f);
        test_image_with_values[0] = 255.0f;
        test_image_with_values[100] = 128.0f;
    }

    std::vector<std::string> test_model_paths;
    std::vector<std::string> main_model_paths;
    std::vector<float> test_image;
    std::vector<float> test_image_with_values;
};

TEST_F(MLPModelTest, LoadModelSuccess) {
    // Проверяем существование файлов перед тестом
    std::ifstream w1_file(test_model_paths[0]);
    std::ifstream w2_file(test_model_paths[1]);
    if (!w1_file.is_open() || !w2_file.is_open()) {
        GTEST_SKIP() << "Test weight files not found";
    }
    
    MLPModel model;
    EXPECT_TRUE(model.loadModel(test_model_paths));
    EXPECT_TRUE(model.isLoaded());
}

TEST_F(MLPModelTest, LoadMainModelSuccess) {
    // Проверяем существование основных файлов перед тестом
    std::ifstream w1_file(main_model_paths[0]);
    std::ifstream w2_file(main_model_paths[1]);
    if (!w1_file.is_open() || !w2_file.is_open()) {
        GTEST_SKIP() << "Main weight files not found";
    }
    
    MLPModel model;
    EXPECT_TRUE(model.loadModel(main_model_paths));
    EXPECT_TRUE(model.isLoaded());
}


TEST_F(MLPModelTest, LoadModelFileNotFound) {
    MLPModel model;
    std::vector<std::string> invalid_paths = {"nonexistent_w1.txt", "nonexistent_w2.txt"};
    
    EXPECT_FALSE(model.loadModel(invalid_paths));
    EXPECT_FALSE(model.isLoaded());
}

TEST_F(MLPModelTest, PredictWithoutLoading) {
    MLPModel model;
    
    EXPECT_THROW({
        model.predict(test_image);
    }, std::runtime_error);
}

