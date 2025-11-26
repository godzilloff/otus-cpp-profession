/**
 * @file test_reducers.cpp
 * @brief Unit tests for MapReduce reducers
 * 
 * Contains Google Test cases for testing reducer functionality
 * including mean reducer and variance reducer.
 */

#include <gtest/gtest.h>
#include <cmath>
#include <iostream>
#include "test_utils.h"

class ReducerTest : public ::testing::Test {
protected:
    void SetUp() override {
        #ifdef _WIN32
            reducer_mean_exe = "bin/reducer_mean.exe";
            reducer_variance_exe = "bin/reducer_variance.exe";
        #else
            reducer_mean_exe = "bin/reducer_mean";
            reducer_variance_exe = "bin/reducer_variance";
        #endif
        
        std::cout << "Reducer mean path: " << reducer_mean_exe << std::endl;
        std::cout << "Reducer variance path: " << reducer_variance_exe << std::endl;
        
        // Check if executables exist
        if (!mapreduce_test::executable_exists(reducer_mean_exe)) {
            std::cerr << "WARNING: " << reducer_mean_exe << " not found!" << std::endl;
        }
        if (!mapreduce_test::executable_exists(reducer_variance_exe)) {
            std::cerr << "WARNING: " << reducer_variance_exe << " not found!" << std::endl;
        }
    }
    
    std::string reducer_mean_exe;
    std::string reducer_variance_exe;
};

/**
 * @test Test variance reducer with single value
 */
TEST_F(ReducerTest, VarianceReducerSingleValue) {
    // Skip test if executable doesn't exist
    if (!mapreduce_test::executable_exists(reducer_variance_exe)) {
        GTEST_SKIP() << "Reducer executable not found, skipping test";
    }
    
    std::string input = "variance\t100\t10000\n"; // price and price^2
    
    std::string output = mapreduce_test::run_reducer(reducer_variance_exe, input);
    
    std::stringstream ss(output);
    std::string line;
    std::vector<std::string> lines;
    
    while (std::getline(ss, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    
    EXPECT_GE(lines.size(), 2) << "Expected at least 2 output lines, got " << lines.size() << ". Output: " << output;
    
    // Find variance line
    bool found_variance = false;
    for (const auto& result_line : lines) {
        if (result_line.find("variance") == 0) {
            auto variance_result = mapreduce_test::parse_output_line(result_line);
            EXPECT_EQ(variance_result.first, "variance");
            EXPECT_NEAR(variance_result.second, 0.0, 1e-6) << "Variance should be 0 for single value";
            found_variance = true;
            break;
        }
    }
    
    EXPECT_TRUE(found_variance) << "Variance line not found in output: " << output;
}
