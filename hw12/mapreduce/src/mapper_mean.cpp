/**
 * @file mapper_mean.cpp
 * @brief Mapper for calculating mean price of NYC real estate
 */

#include <iostream>
#include <string>
#include <sstream>

/**
 * @brief Main function for mean price mapper
 * 
 * Reads CSV data from standard input, extracts price from 10th column,
 * and emits key-value pairs for the reducer.
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return int Exit status (0 for success)
 */
int main(int, char** ) {
    std::string line;
    
    // Process each line from standard input
    while (std::getline(std::cin, line)) {
        std::stringstream ss(line);
        std::string token;
        int column_count = 0;
        double price = 0.0;
        
        // Parse CSV line
        while (std::getline(ss, token, ',')) {
            column_count++;
            
            // Price is in the 10th column (index 9)
            if (column_count == 10) {
                try {
                    price = std::stod(token);
                    // Emit key-value pair: "price" and actual price
                    std::cout << "price\t" << price << std::endl;
                } catch (const std::exception&) {
                    // Skip invalid price data
                    continue;
                }
                break;
            }
        }
    }
    
    return 0;
}
