/**
 * @file mapper_variance.cpp
 * @brief Mapper for calculating variance of NYC real estate prices
 * 
 * This mapper processes CSV data and extracts price information from the 10th column.
 * It outputs key-value pairs where key is "variance" and values are price and price squared.
 * This provides the necessary data for calculating variance using the formula:
 * Variance = E[X²] - (E[X])²
 */

#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

/**
 * @brief Main function for variance mapper
 * 
 * Reads CSV data from standard input, extracts price from 10th column,
 * and emits key-value pairs containing both price and price squared for the reducer.
 * 
 * @param argc Argument count
 * @param argv Argument vector  
 * @return int Exit status (0 for success)
 */
int main(int, char**) {
    std::string line;
    
    // Process each line from standard input
    while (std::getline(std::cin, line)) {
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        std::stringstream ss(line);
        std::string token;
        int column_count = 0;
        double price = 0.0;
        bool valid_price = false;
        
        // Parse CSV line
        while (std::getline(ss, token, ',')) {
            column_count++;
            
            // Price is in the 10th column (index 9)
            if (column_count == 10) {
                try {
                    price = std::stod(token);
                    // Validate that price is positive and reasonable
                    if (price > 0 && price < 100000) { // Reasonable price range for NYC
                        valid_price = true;
                    }
                } catch (const std::exception& ) {
                    // Skip invalid price data
                    valid_price = false;
                }
                break;
            }
        }
        
        // Emit key-value pair if price is valid
        if (valid_price) {
            double price_squared = price * price;
            // Emit: key = "variance", value1 = price, value2 = price²
            std::cout << "variance\t" << price << "\t" << price_squared << std::endl;
        }
    }
    
    return 0;
}
