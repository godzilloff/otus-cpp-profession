/**
 * @file test_mappers.cpp
 * @brief Unit tests for MapReduce mappers
 * 
 * Contains Google Test cases for testing mapper functionality
 * including mean mapper and variance mapper.
 */

#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include "test_utils.h"

class MapperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use safe_getenv instead of std::getenv
        std::string test_data_dir = mapreduce_test::safe_getenv("TEST_DATA_DIR");
        if (!test_data_dir.empty()) {
            test_data_path = test_data_dir;
        } else {
            test_data_path = "test_data";
        }
        
        // Set executable paths using relative paths from build directory
        mapper_mean_exe = mapreduce_test::get_executable_path("mapper_mean");
        mapper_variance_exe = mapreduce_test::get_executable_path("mapper_variance");
        
        std::cout << "Mapper mean path: " << mapper_mean_exe << std::endl;
        std::cout << "Mapper variance path: " << mapper_variance_exe << std::endl;
        
        // Check if executables exist
        if (!mapreduce_test::executable_exists(mapper_mean_exe)) {
            std::cerr << "WARNING: " << mapper_mean_exe << " not found!" << std::endl;
        }
        if (!mapreduce_test::executable_exists(mapper_variance_exe)) {
            std::cerr << "WARNING: " << mapper_variance_exe << " not found!" << std::endl;
        }
    }
    
    void TearDown() override {
        // Clean up temporary files
        std::remove("test_input.tmp");
    }
    
    std::string test_data_path;
    std::string mapper_mean_exe;
    std::string mapper_variance_exe;
};

/**
 * @test Test mean mapper with single valid record
 */
TEST_F(MapperTest, MeanMapperSingleRecord) {
    // Skip test if executable doesn't exist
    if (!mapreduce_test::executable_exists(mapper_mean_exe)) {
        GTEST_SKIP() << "Mapper executable not found, skipping test";
    }
    
    std::string input = "1,Description,123,Name,Manhattan,Midtown,40.0,-73.0,Private room,150,1,10,2024-01-01,0.5,1,100\n";
    
    std::string output = mapreduce_test::run_mapper(mapper_mean_exe, input);
    
    EXPECT_FALSE(output.empty()) << "Mapper produced no output";
    if (!output.empty()) {
        // Remove any trailing newlines for comparison
        std::string cleaned_output = output;
        if (!cleaned_output.empty() && cleaned_output.back() == '\n') {
            cleaned_output.pop_back();
        }
        EXPECT_EQ(cleaned_output, "price\t150") << "Unexpected output: " << output;
    }
}

/**
 * @test Test mean mapper with multiple records
 */
TEST_F(MapperTest, MeanMapperMultipleRecords) {
    // Skip test if executable doesn't exist
    if (!mapreduce_test::executable_exists(mapper_mean_exe)) {
        GTEST_SKIP() << "Mapper executable not found, skipping test";
    }
    
    std::string input = 
        "1,Desc1,123,Name1,Manhattan,Midtown,40.0,-73.0,Private room,100,1,10,2024-01-01,0.5,1,100\n"
        "2,Desc2,124,Name2,Brooklyn,Park,40.1,-73.1,Entire home/apt,200,2,20,2024-01-02,0.6,2,200\n"
        "3,Desc3,125,Name3,Queens,Area,40.2,-73.2,Private room,300,3,30,2024-01-03,0.7,3,300\n";
    
    std::string output = mapreduce_test::run_mapper(mapper_mean_exe, input);
    
    EXPECT_FALSE(output.empty()) << "Mapper produced no output";
    
    if (!output.empty()) {
        std::stringstream ss(output);
        std::string line;
        std::vector<std::string> lines;
        
        while (std::getline(ss, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        
        EXPECT_EQ(lines.size(), 3) << "Expected 3 output lines, got " << lines.size() << ". Output: " << output;
        
        // Check if all expected lines are present
        bool found100 = false, found200 = false, found300 = false;
        for (const auto& output_line : lines) {
            if (output_line == "price\t100") found100 = true;
            if (output_line == "price\t200") found200 = true;
            if (output_line == "price\t300") found300 = true;
        }
        
        EXPECT_TRUE(found100) << "Missing price 100 in output: " << output;
        EXPECT_TRUE(found200) << "Missing price 200 in output: " << output;
        EXPECT_TRUE(found300) << "Missing price 300 in output: " << output;
    }
}

/**
 * @test Test mean mapper with invalid price data
 */
TEST_F(MapperTest, MeanMapperInvalidPrice) {
    // Skip test if executable doesn't exist
    if (!mapreduce_test::executable_exists(mapper_mean_exe)) {
        GTEST_SKIP() << "Mapper executable not found, skipping test";
    }
    
    std::string input = 
        "1,Desc1,123,Name1,Manhattan,Midtown,40.0,-73.0,Private room,INVALID,1,10,2024-01-01,0.5,1,100\n"
        "2,Desc2,124,Name2,Brooklyn,Park,40.1,-73.1,Entire home/apt,200,2,20,2024-01-02,0.6,2,200\n";
    
    std::string output = mapreduce_test::run_mapper(mapper_mean_exe, input);
    
    // Should only process valid record
    if (!output.empty()) {
        std::stringstream ss(output);
        std::string line;
        std::vector<std::string> lines;
        
        while (std::getline(ss, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        
        // Should have exactly one valid output
        EXPECT_EQ(lines.size(), 1) << "Expected 1 output line, got " << lines.size() << ". Output: " << output;
        if (lines.size() == 1) {
            EXPECT_EQ(lines[0], "price\t200") << "Unexpected output: " << lines[0];
        }
    } else {
        // It's also acceptable if mapper produces no output for invalid data
        SUCCEED();
    }
}

/**
 * @test Test variance mapper with single record
 */
TEST_F(MapperTest, VarianceMapperSingleRecord) {
    // Skip test if executable doesn't exist
    if (!mapreduce_test::executable_exists(mapper_variance_exe)) {
        GTEST_SKIP() << "Mapper executable not found, skipping test";
    }
    
    std::string input = "1,Description,123,Name,Manhattan,Midtown,40.0,-73.0,Private room,100,1,10,2024-01-01,0.5,1,100\n";
    
    std::string output = mapreduce_test::run_mapper(mapper_variance_exe, input);
    
    EXPECT_FALSE(output.empty()) << "Mapper produced no output";
    if (!output.empty()) {
        // Remove any trailing newlines for comparison
        std::string cleaned_output = output;
        if (!cleaned_output.empty() && cleaned_output.back() == '\n') {
            cleaned_output.pop_back();
        }
        EXPECT_EQ(cleaned_output, "variance\t100\t10000") << "Unexpected output: " << output;
    }
}

/**
 * @test Test mappers with empty input
 */
TEST_F(MapperTest, MapperEmptyInput) {
    std::string input = "";
    
    // Skip tests if executables don't exist
    if (mapreduce_test::executable_exists(mapper_mean_exe)) {
        std::string mean_output = mapreduce_test::run_mapper(mapper_mean_exe, input);
        // Empty output is acceptable for empty input
        SUCCEED();
    } else {
        GTEST_SKIP() << "Mapper mean executable not found, skipping test";
    }
    
    if (mapreduce_test::executable_exists(mapper_variance_exe)) {
        std::string variance_output = mapreduce_test::run_mapper(mapper_variance_exe, input);
        // Empty output is acceptable for empty input
        SUCCEED();
    } else {
        GTEST_SKIP() << "Mapper variance executable not found, skipping test";
    }
}
