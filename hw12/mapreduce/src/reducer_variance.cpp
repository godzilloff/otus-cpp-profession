/**
 * @file reducer_variance.cpp
 * @brief Reducer for calculating variance and standard deviation of NYC real estate prices
 * 
 * This reducer processes key-value pairs from mapper and calculates:
 * - Variance using the formula: Var(X) = E[X²] - (E[X])²
 * - Standard deviation as sqrt(Variance)
 * 
 * The reducer aggregates:
 * - Sum of all prices (for calculating mean)
 * - Sum of all squared prices (for calculating E[X²])
 * - Count of records (for normalization)
 */

#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>

/**
 * @brief Main function for variance reducer
 * 
 * Aggregates price and price² values from mapper output and calculates:
 * - Mean price
 * - Variance
 * - Standard deviation
 * 
 * Input format: "variance\t<price>\t<price_squared>"
 * Output format: 
 *   "mean_price\t<mean_value>"
 *   "variance\t<variance_value>"
 *   "std_deviation\t<std_deviation_value>"
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return int Exit status (0 for success)
 */
int main(int, char**) {
    std::string line;
    std::string current_key;
    double sum_prices = 0.0;
    double sum_squares = 0.0;
    long long count = 0;
    
    // Process each line from standard input
    while (std::getline(std::cin, line)) {
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        std::stringstream ss(line);
        std::string key;
        double price, square;
        
        // Parse key-value pair: "variance\tprice\tprice_squared"
        if (ss >> key >> price >> square) {
            if (current_key.empty()) {
                current_key = key;
            }
            
            // Aggregate values for the same key
            if (key == current_key) {
                sum_prices += price;
                sum_squares += square;
                count++;
            }
        }
    }
    
    // Calculate and output statistics if we have valid data
    if (count > 0) {
        double mean = sum_prices / count;
        double mean_of_squares = sum_squares / count;
        double variance = mean_of_squares - (mean * mean);
        
        // Ensure variance is non-negative (handles floating point precision issues)
        if (variance < 0) {
            variance = 0.0;
        }
        
        double std_deviation = std::sqrt(variance);
        
        // Set precision for floating point output
        std::cout << std::fixed << std::setprecision(2);
        
        // Output results
        std::cout << "mean_price\t" << mean << std::endl;
        std::cout << "variance\t" << variance << std::endl;
        std::cout << "std_deviation\t" << std_deviation << std::endl;
    } else {
        std::cerr << "Error: No valid data processed" << std::endl;
        return 1;
    }
    
    return 0;
}
