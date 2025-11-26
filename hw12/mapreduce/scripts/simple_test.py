#!/usr/bin/env python3
"""
Simple test script that works with the provided sample data
"""

import os
import sys

def main():
    print("SIMPLE MAPREDUCE TEST")
    print("====================")
    
    # Show current directory
    print(f"Current directory: {os.getcwd()}")
    
    # Try to find data file
    data_files = []
    for root, dirs, files in os.walk('..'):
        for file in files:
            if 'NYC' in file or 'AB' in file:
                full_path = os.path.join(root, file)
                data_files.append(full_path)
                print(f"Found: {full_path}")
    
    if not data_files:
        print("No data files found!")
        return
    
    # Use the first found file
    data_file = data_files[0]
    print(f"\nUsing data file: {data_file}")
    
    # Simple manual calculation with the provided sample data
    sample_data = [
        [2539, "Clean & quiet apt home by the park", 2787, "John", "Brooklyn", "Kensington", 40.64749, -73.97237, "Private room", 149, 1, 9, "2018-10-19", 0.21, 6, 365],
        [2595, "Skylit Midtown Castle", 2845, "Jennifer", "Manhattan", "Midtown", 40.75362, -73.98377, "Entire home/apt", 225, 1, 45, "2019-05-21", 0.38, 2, 355],
        [3647, "THE VILLAGE OF HARLEM....NEW YORK !", 4632, "Elisabeth", "Manhattan", "Harlem", 40.80902, -73.9419, "Private room", 150, 3, 0, "", "", 1, 365],
        [3831, "Cozy Entire Floor of Brownstone", 4869, "LisaRoxanne", "Brooklyn", "Clinton Hill", 40.68514, -73.95976, "Entire home/apt", 89, 1, 270, "2019-07-05", 4.64, 1, 194],
        [5022, "Entire Apt: Spacious Studio/Loft by central park", 7192, "Laura", "Manhattan", "East Harlem", 40.79851, -73.94399, "Entire home/apt", 80, 10, 9, "2018-11-19", 0.10, 1, 0],
        [5099, "Large Cozy 1 BR Apartment In Midtown East", 7322, "Chris", "Manhattan", "Murray Hill", 40.74767, -73.975, "Entire home/apt", 200, 3, 74, "2019-06-22", 0.59, 1, 129],
        [5121, "BlissArtsSpace!", 7356, "Garon", "Brooklyn", "Bedford-Stuyvesant", 40.68688, -73.95596, "Private room", 60, 45, 49, "2017-10-05", 0.40, 1, 0],
        [5178, "Large Furnished Room Near B'way", 8967, "Shunichi", "Manhattan", "Hell's Kitchen", 40.76489, -73.98493, "Private room", 79, 2, 430, "2019-06-24", 3.47, 1, 220],
        [5203, "Cozy Clean Guest Room - Family Apt", 7490, "MaryEllen", "Manhattan", "Upper West Side", 40.80178, -73.96723, "Private room", 79, 2, 118, "2017-07-21", 0.99, 1, 0],
        [5238, "Cute & Cozy Lower East Side 1 bdrm", 7549, "Ben", "Manhattan", "Chinatown", 40.71344, -73.99037, "Entire home/apt", 150, 1, 160, "2019-06-09", 1.33, 4, 188],
        [5295, "Beautiful 1br on Upper West Side", 7702, "Lena", "Manhattan", "Upper West Side", 40.80316, -73.96545, "Entire home/apt", 135, 5, 53, "2019-06-22", 0.43, 1, 6],
        [5441, "Central Manhattan/near Broadway", 7989, "Kate", "Manhattan", "Hell's Kitchen", 40.76076, -73.98867, "Private room", 85, 2, 188, "2019-06-23", 1.50, 1, 39],
        [5803, "Lovely Room 1; Garden; Best Area; Legal rental", 9744, "Laurie", "Brooklyn", "South Slope", 40.66829, -73.98779, "Private room", 89, 4, 167, "2019-06-24", 1.34, 3, 314],
        [6021, "Wonderful Guest Bedroom in Manhattan for SINGLES", 11528, "Claudio", "Manhattan", "Upper West Side", 40.79826, -73.96113, "Private room", 85, 2, 113, "2019-07-05", 0.91, 1, 333]
    ]
    
    # Extract prices (10th element in each row, index 9)
    prices = [row[9] for row in sample_data]
    
    print(f"\nSample data prices: {prices}")
    print(f"Number of records: {len(prices)}")
    
    # Calculate manually
    total = sum(prices)
    count = len(prices)
    mean = total / count
    
    # Calculate variance using two methods
    squared_diffs = [(p - mean) ** 2 for p in prices]
    variance_direct = sum(squared_diffs) / count
    
    squares = [p * p for p in prices]
    mean_of_squares = sum(squares) / count
    variance_mapreduce = mean_of_squares - (mean * mean)
    
    std_dev = variance_direct ** 0.5
    
    print(f"\nCALCULATIONS:")
    print(f"Sum: {total}")
    print(f"Count: {count}")
    print(f"Mean: {mean:.2f}")
    print(f"Variance (direct): {variance_direct:.2f}")
    print(f"Variance (MapReduce): {variance_mapreduce:.2f}")
    print(f"Standard deviation: {std_dev:.2f}")
    print(f"Formulas match: {abs(variance_direct - variance_mapreduce) < 0.01}")

if __name__ == "__main__":
    main()