/**
 * @file test_utils.h
 * @brief Utility functions for MapReduce tests
 * 
 * Contains helper functions for testing mappers and reducers,
 * including file I/O operations and result validation.
 */

#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <cmath>
#include <algorithm>

namespace mapreduce_test {

// Объявления функций
std::string safe_getenv(const std::string& name);
std::string get_executable_path(const std::string& exe_name);
bool executable_exists(const std::string& exe_path);
std::string run_mapper(const std::string& mapper_executable, const std::string& input_data);
std::string run_reducer(const std::string& reducer_executable, const std::string& input_data);
std::string run_mapreduce(const std::string& mapper_executable, 
                         const std::string& reducer_executable, 
                         const std::string& input_data);
std::string read_file(const std::string& filename);
std::pair<std::string, double> parse_output_line(const std::string& output);
std::vector<double> parse_output_values(const std::string& output);
bool approximately_equal(double a, double b, double epsilon = 1e-6);

} // namespace mapreduce_test
