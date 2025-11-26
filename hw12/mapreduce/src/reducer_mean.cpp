/**
 * @file reducer_mean.cpp
 * @brief Reducer for calculating mean price of NYC real estate
 * 
 * This reducer processes key-value pairs from mapper and calculates
 * the mean price by summing all prices and dividing by count.
 */

#include <iostream>
#include <string>
#include <sstream>

/**
 * @brief Main function for mean price reducer
 * 
 * Aggregates price values from mapper output and calculates mean price.
 * Input format: "price\t<value>"
 * Output format: "mean_price\t<mean_value>"
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return int Exit status (0 for success)
 */
int main(int , char** ) {
    std::string line;
    std::string current_key;
    double sum = 0.0;
    int count = 0;
    
    // Process each line from standard input
    while (std::getline(std::cin, line)) {
        std::stringstream ss(line);
        std::string key;
        double value;
        
        // Parse key-value pair
        ss >> key >> value;
        
        if (current_key.empty()) {
            current_key = key;
        }
        
        // Aggregate values for the same key
        if (key == current_key) {
            sum += value;
            count++;
        }
    }
    
    // Calculate and output mean price
    if (count > 0) {
        double mean = sum / count;
        std::cout << "mean_price\t" << mean << std::endl;
    }
    
    return 0;
}
