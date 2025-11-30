/**
 * @file test_csv_reader.cpp
 * @brief Тесты для CSVReader
 */

#include <gtest/gtest.h>
#include "CSVReader.h"
#include <filesystem>

class CSVReaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Используем пути относительно бинарной директории
        test_data_path = "test_data/small_test.csv";
        weights_path = "test_data/sample_weights_w1.txt";
        main_test_data_path = "test_data/test.csv";  // Для основных тестов
        main_weights_path = "test_data/w1.txt";     // Для основных тестов
    }

    std::string test_data_path;
    std::string weights_path;
    std::string main_test_data_path;
    std::string main_weights_path;
};

TEST_F(CSVReaderTest, ReadCSVFileExists) {
    // Проверяем существование файла перед тестом
    std::ifstream test_file(test_data_path);
    if (!test_file.is_open()) {
        GTEST_SKIP() << "Test data file not found: " << test_data_path;
    }
    
    EXPECT_NO_THROW({
        auto data = CSVReader::readCSV(test_data_path);
        EXPECT_FALSE(data.empty());
    });
}

TEST_F(CSVReaderTest, ReadMainCSVFileExists) {
    // Проверяем существование основного файла перед тестом
    std::ifstream test_file(main_test_data_path);
    if (!test_file.is_open()) {
        GTEST_SKIP() << "Main test data file not found: " << main_test_data_path;
    }
    
    EXPECT_NO_THROW({
        auto data = CSVReader::readCSV(main_test_data_path);
        EXPECT_FALSE(data.empty());
    });
}
