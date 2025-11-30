/**
 * @file test_integration.cpp
 * @brief Интеграционные тесты
 */

#include <gtest/gtest.h>
#include "MLPModel.h"
#include "CSVReader.h"

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_model_paths = {
            "test_data/sample_weights_w1.txt",
            "test_data/sample_weights_w2.txt"
        };
        test_csv_path = "test_data/small_test.csv";
        main_model_paths = {
            "test_data/w1.txt",
            "test_data/w2.txt"
        };
        main_csv_path = "test_data/test.csv";
    }

    std::vector<std::string> test_model_paths;
    std::string test_csv_path;
    std::vector<std::string> main_model_paths;
    std::string main_csv_path;
};

TEST_F(IntegrationTest, FullPipeline) {
    // Проверяем существование файлов перед тестом
    std::ifstream csv_file(test_csv_path);
    std::ifstream w1_file(test_model_paths[0]);
    std::ifstream w2_file(test_model_paths[1]);
    if (!csv_file.is_open() || !w1_file.is_open() || !w2_file.is_open()) {
        GTEST_SKIP() << "Test files not found";
    }
    
    // Загружаем данные
    auto test_data = CSVReader::readCSV(test_csv_path);
    ASSERT_FALSE(test_data.empty());
    
    // Загружаем модель
    MLPModel model;
    ASSERT_TRUE(model.loadModel(test_model_paths));
    
    // Предсказываем
    std::vector<std::vector<float>> images;
    std::vector<int> true_labels;
    
    for (const auto& sample : test_data) {
        images.push_back(sample.features);
        true_labels.push_back(sample.label);
    }
    
    auto predictions = model.predictBatch(images);
    EXPECT_EQ(predictions.size(), true_labels.size());
}

TEST_F(IntegrationTest, MainPipeline) {
    // Проверяем существование основных файлов перед тестом
    std::ifstream csv_file(main_csv_path);
    std::ifstream w1_file(main_model_paths[0]);
    std::ifstream w2_file(main_model_paths[1]);
    if (!csv_file.is_open() || !w1_file.is_open() || !w2_file.is_open()) {
        GTEST_SKIP() << "Main files not found";
    }
    
    // Загружаем данные
    auto test_data = CSVReader::readCSV(main_csv_path);
    ASSERT_FALSE(test_data.empty());
    
    // Загружаем модель
    MLPModel model;
    ASSERT_TRUE(model.loadModel(main_model_paths));
    
    // Предсказываем
    std::vector<std::vector<float>> images;
    std::vector<int> true_labels;
    
    for (const auto& sample : test_data) {
        images.push_back(sample.features);
        true_labels.push_back(sample.label);
    }
    
    auto predictions = model.predictBatch(images);
    EXPECT_EQ(predictions.size(), true_labels.size());
    
    // Вычисляем accuracy
    size_t correct = 0;
    for (size_t i = 0; i < true_labels.size(); i++) {
        if (true_labels[i] == predictions[i]) {
            correct++;
        }
    }
    double accuracy = static_cast<double>(correct) / true_labels.size();
    std::cout << "Accuracy on main data: " << accuracy << std::endl;
}
