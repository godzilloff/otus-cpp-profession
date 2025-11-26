/**
 * @file test_integration.cpp
 * @brief Integration tests for MapReduce pipeline
 * 
 * Contains end-to-end tests that verify the complete MapReduce
 * pipeline works correctly with sample data.
 */

#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include "test_utils.h"

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use safe_getenv instead of std::getenv
        std::string test_data_dir = mapreduce_test::safe_getenv("TEST_DATA_DIR");
        if (!test_data_dir.empty()) {
            test_data_path = test_data_dir;
        } else {
            test_data_path = "test_data";
        }
        
        // Set executable paths
        mapper_mean_exe = mapreduce_test::get_executable_path("mapper_mean");
        reducer_mean_exe = mapreduce_test::get_executable_path("reducer_mean");
        mapper_variance_exe = mapreduce_test::get_executable_path("mapper_variance");
        reducer_variance_exe = mapreduce_test::get_executable_path("reducer_variance");
        
        std::cout << "Integration test executables:" << std::endl;
        std::cout << "  Mapper mean: " << mapper_mean_exe << std::endl;
        std::cout << "  Reducer mean: " << reducer_mean_exe << std::endl;
        std::cout << "  Mapper variance: " << mapper_variance_exe << std::endl;
        std::cout << "  Reducer variance: " << reducer_variance_exe << std::endl;
    }
    
    std::string test_data_path;
    std::string mapper_mean_exe;
    std::string reducer_mean_exe;
    std::string mapper_variance_exe;
    std::string reducer_variance_exe;
};

/**
 * @test Test complete mean calculation pipeline with sample data
 */
TEST_F(IntegrationTest, CompleteMeanPipeline) {
    // Skip test if executables don't exist
    if (!mapreduce_test::executable_exists(mapper_mean_exe) || 
        !mapreduce_test::executable_exists(reducer_mean_exe)) {
        GTEST_SKIP() << "Required executables not found, skipping test";
    }
    
    std::string sample_input = 
        "1,Cozy apartment,1001,John,Manhattan,Midtown,40.0,-73.0,Private room,100,1,10,2024-01-01,0.5,1,100\n"
        "2,Luxury condo,1002,Jane,Brooklyn,Park,40.1,-73.1,Entire home/apt,200,2,20,2024-01-02,0.6,2,200\n"
        "3,Comfortable room,1003,Bob,Queens,Astoria,40.2,-73.2,Private room,150,1,15,2024-01-03,0.15,1,300\n";
    
    std::string output = mapreduce_test::run_mapreduce(
        mapper_mean_exe, reducer_mean_exe, sample_input);
    
    EXPECT_FALSE(output.empty()) << "MapReduce pipeline produced no output";
    
    if (!output.empty()) {
        auto result = mapreduce_test::parse_output_line(output);
        EXPECT_EQ(result.first, "mean_price");
        // Expected mean of sample data: (100+200+150)/3 = 150
        EXPECT_NEAR(result.second, 150.0, 0.001);
    }
}

/**
 * @test Test complete variance calculation pipeline with sample data
 */
TEST_F(IntegrationTest, CompleteVariancePipeline) {
    // Skip test if executables don't exist
    if (!mapreduce_test::executable_exists(mapper_variance_exe) || 
        !mapreduce_test::executable_exists(reducer_variance_exe)) {
        GTEST_SKIP() << "Required executables not found, skipping test";
    }
    
    std::string sample_input = 
        "1,Cozy apartment,1001,John,Manhattan,Midtown,40.0,-73.0,Private room,100,1,10,2024-01-01,0.5,1,100\n"
        "2,Luxury condo,1002,Jane,Brooklyn,Park,40.1,-73.1,Entire home/apt,200,2,20,2024-01-02,0.6,2,200\n"
        "3,Comfortable room,1003,Bob,Queens,Astoria,40.2,-73.2,Private room,150,1,15,2024-01-03,0.15,1,300\n";
    
    std::string output = mapreduce_test::run_mapreduce(
        mapper_variance_exe, reducer_variance_exe, sample_input);
    
    EXPECT_FALSE(output.empty()) << "MapReduce pipeline produced no output";
    
    if (!output.empty()) {
        std::stringstream ss(output);
        std::string line;
        
        // Parse all output lines
        double mean = 0.0, variance = 0.0, std_deviation = 0.0;
        
        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            
            std::stringstream line_ss(line);
            std::string key;
            double value;
            
            if (line_ss >> key >> value) {
                if (key == "mean_price") {
                    mean = value;
                } else if (key == "variance") {
                    variance = value;
                } else if (key == "std_deviation") {
                    std_deviation = value;
                }
            }
        }
        
        // Verify calculations
        EXPECT_NEAR(mean, 150.0, 0.01);
        
        // Check variance
        // Prices: 100, 200, 150
        // Mean: 150
        // Variance: ((100-150)^2 + (200-150)^2 + (150-150)^2)/3 = (2500+2500+0)/3 ≈ 1666.67
        EXPECT_NEAR(variance, 1666.666, 0.1);
        
        // Check standard deviation
        EXPECT_NEAR(std_deviation, std::sqrt(variance), 0.01);
    }
}
