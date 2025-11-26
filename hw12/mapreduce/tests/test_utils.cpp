/**
 * @file test_utils.cpp
 * @brief Implementation of utility functions for MapReduce tests
 * 
 * Contains implementation of helper functions for testing mappers and reducers.
 */

#include "test_utils.h"
#include <iostream>

#ifdef _WIN32
    #include <io.h>
    #define popen _popen
    #define pclose _pclose
    // Safe version of getenv for Windows
    #include <stdlib.h>
    #include <errno.h>
#else
    #include <unistd.h>
#endif

namespace mapreduce_test {

/**
 * @brief Safe cross-platform getenv function
 * 
 * @param name Environment variable name
 * @return std::string Environment variable value or empty string if not found
 */
std::string safe_getenv(const std::string& name) {
#ifdef _WIN32
    char* buffer = nullptr;
    size_t size = 0;
    if (_dupenv_s(&buffer, &size, name.c_str()) == 0 && buffer != nullptr) {
        std::string result(buffer);
        free(buffer);
        return result;
    }
    return "";
#else
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : "";
#endif
}

/**
 * @brief Get the full path to executable
 * 
 * @param exe_name Executable name
 * @return std::string Full path to executable
 */
std::string get_executable_path(const std::string& exe_name) {
    std::string base_path = "bin/";
    
    #ifdef _WIN32
        return base_path + exe_name + ".exe";
    #else
        return base_path + exe_name;
    #endif
}

/**
 * @brief Check if executable exists
 * 
 * @param exe_path Path to executable
 * @return true If executable exists
 */
bool executable_exists(const std::string& exe_path) {
    std::ifstream file(exe_path);
    return file.good();
}

/**
 * @brief Run a mapper with given input and return output
 * 
 * @param mapper_executable Path to mapper executable
 * @param input_data Input data as string
 * @return std::string Output from mapper
 */
std::string run_mapper(const std::string& mapper_executable, const std::string& input_data) {
    // Check if executable exists
    if (!executable_exists(mapper_executable)) {
        std::cerr << "Error: Executable not found: " << mapper_executable << std::endl;
        return "";
    }
    
    // Create input file
    std::string input_file = "test_input.tmp";
    std::ofstream input_stream(input_file);
    if (!input_stream) {
        throw std::runtime_error("Cannot create input file: " + input_file);
    }
    input_stream << input_data;
    input_stream.close();
    
    // Run command
    std::string full_command;
    #ifdef _WIN32
        full_command = "type " + input_file + " | \"" + mapper_executable + "\"";
    #else
        full_command = "cat " + input_file + " | " + mapper_executable;
    #endif
    
    std::cout << "Running command: " << full_command << std::endl;
    
    FILE* pipe = popen(full_command.c_str(), "r");
    if (!pipe) {
        std::remove(input_file.c_str());
        throw std::runtime_error("Failed to run mapper with command: " + full_command);
    }
    
    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        std::cerr << "Command failed with status: " << status << std::endl;
    }
    
    std::remove(input_file.c_str());
    
    return result;
}

/**
 * @brief Run a reducer with given input and return output
 * 
 * @param reducer_executable Path to reducer executable
 * @param input_data Input data as string (should be sorted)
 * @return std::string Output from reducer
 */
std::string run_reducer(const std::string& reducer_executable, const std::string& input_data) {
    // Check if executable exists
    if (!executable_exists(reducer_executable)) {
        std::cerr << "Error: Executable not found: " << reducer_executable << std::endl;
        return "";
    }
    
    // Create input file
    std::string input_file = "test_input.tmp";
    std::ofstream input_stream(input_file);
    if (!input_stream) {
        throw std::runtime_error("Cannot create input file: " + input_file);
    }
    input_stream << input_data;
    input_stream.close();
    
    // Run command
    std::string full_command;
    #ifdef _WIN32
        full_command = "type " + input_file + " | \"" + reducer_executable + "\"";
    #else
        full_command = "cat " + input_file + " | " + reducer_executable;
    #endif
    
    std::cout << "Running command: " << full_command << std::endl;
    
    FILE* pipe = popen(full_command.c_str(), "r");
    if (!pipe) {
        std::remove(input_file.c_str());
        throw std::runtime_error("Failed to run reducer with command: " + full_command);
    }
    
    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        std::cerr << "Command failed with status: " << status << std::endl;
    }
    
    std::remove(input_file.c_str());
    
    return result;
}

/**
 * @brief Run full MapReduce pipeline
 * 
 * @param mapper_executable Path to mapper executable
 * @param reducer_executable Path to reducer executable
 * @param input_data Input data as string
 * @return std::string Final output
 */
std::string run_mapreduce(const std::string& mapper_executable, 
                         const std::string& reducer_executable, 
                         const std::string& input_data) {
    // Run mapper
    std::string mapper_output = run_mapper(mapper_executable, input_data);
    if (mapper_output.empty()) {
        std::cerr << "Mapper produced no output" << std::endl;
        return "";
    }
    
    // Sort mapper output (simulate shuffle/sort phase)
    std::stringstream mapper_stream(mapper_output);
    std::vector<std::string> lines;
    std::string line;
    
    while (std::getline(mapper_stream, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    
    std::sort(lines.begin(), lines.end());
    
    std::string sorted_output;
    for (const auto& sorted_line : lines) {
        sorted_output += sorted_line + "\n";
    }
    
    // Run reducer
    return run_reducer(reducer_executable, sorted_output);
}

/**
 * @brief Read file content as string
 * 
 * @param filename Path to file
 * @return std::string File content
 */
std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/**
 * @brief Parse key-value pair from reducer output
 * 
 * @param output Reducer output line
 * @return std::pair<std::string, double> Key and value
 */
std::pair<std::string, double> parse_output_line(const std::string& output) {
    std::stringstream ss(output);
    std::string key;
    double value;
    
    ss >> key >> value;
    return {key, value};
}

/**
 * @brief Parse multiple values from reducer output line
 * 
 * @param output Reducer output line
 * @return std::vector<double> Extracted values
 */
std::vector<double> parse_output_values(const std::string& output) {
    std::stringstream ss(output);
    std::string key;
    std::vector<double> values;
    double value;
    
    ss >> key; // Skip the key
    while (ss >> value) {
        values.push_back(value);
    }
    
    return values;
}

/**
 * @brief Check if double values are approximately equal
 * 
 * @param a First value
 * @param b Second value
 * @param epsilon Tolerance
 * @return true If values are equal within tolerance
 */
bool approximately_equal(double a, double b, double epsilon) {
    return std::fabs(a - b) < epsilon;
}

} // namespace mapreduce_test
