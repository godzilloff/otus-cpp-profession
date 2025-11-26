#!/usr/bin/env python3
"""
Test Calculation Script for MapReduce NYC Real Estate Analysis

"""

import pandas as pd
import numpy as np
import os
import sys
from pathlib import Path

def load_and_validate_data(file_path):
    """
    Load and validate the CSV data file.
    
    Args:
        file_path (str): Path to the CSV file
        
    Returns:
        pandas.DataFrame: Loaded and validated data
    """
    try:
        # Check if file exists
        if not os.path.exists(file_path):
            raise FileNotFoundError(f"Data file not found: {file_path}")
        
        # Load CSV file
        print(f"Loading data from: {file_path}")
        df = pd.read_csv(file_path)
        
        # Basic validation
        if df.empty:
            raise ValueError("Data file is empty")
        
        # Check if we have the expected columns
        if len(df.columns) < 10:
            raise ValueError(f"Expected at least 10 columns, got {len(df.columns)}")
        
        print(f"Successfully loaded {len(df)} records")
        print(f"Columns: {list(df.columns)}")
        
        return df
        
    except Exception as e:
        print(f"Error loading data: {e}")
        sys.exit(1)

def extract_prices(df):
    """
    Extract price column from dataframe.
    
    Args:
        df (pandas.DataFrame): Input dataframe
        
    Returns:
        numpy.array: Array of prices
    """
    # Price is in the 10th column (index 9)
    price_column_index = 9
    
    if len(df.columns) <= price_column_index:
        raise ValueError(f"Dataframe doesn't have column at index {price_column_index}")
    
    price_column_name = df.columns[price_column_index]
    print(f"Price column: '{price_column_name}' (index {price_column_index})")
    
    # Extract prices and convert to numeric, handling errors
    prices = pd.to_numeric(df.iloc[:, price_column_index], errors='coerce')
    
    # Remove NaN values (invalid prices)
    valid_prices = prices.dropna()
    
    # Filter out unreasonable prices (negative or too high)
    reasonable_prices = valid_prices[(valid_prices > 0) & (valid_prices < 10000)]
    
    print(f"Total records: {len(df)}")
    print(f"Valid prices: {len(valid_prices)}")
    print(f"Reasonable prices (0 < price < 10000): {len(reasonable_prices)}")
    
    if len(reasonable_prices) == 0:
        raise ValueError("No valid prices found in the dataset")
    
    return reasonable_prices.values

def calculate_statistics(prices):
    """
    Calculate mean, variance and standard deviation.
    
    Args:
        prices (numpy.array): Array of prices
        
    Returns:
        dict: Dictionary with calculated statistics
    """
    # Basic statistics
    mean_price = np.mean(prices)
    variance = np.var(prices, ddof=0)  # Population variance
    std_deviation = np.std(prices, ddof=0)
    
    # Additional statistics for verification
    count = len(prices)
    sum_prices = np.sum(prices)
    sum_squares = np.sum(prices ** 2)
    
    # Calculate variance using MapReduce formula: Var = E[X²] - (E[X])²
    mean_of_squares = np.mean(prices ** 2)
    variance_mapreduce = mean_of_squares - (mean_price ** 2)
    
    statistics = {
        'count': count,
        'sum_prices': sum_prices,
        'sum_squares': sum_squares,
        'mean_price': mean_price,
        'variance': variance,
        'variance_mapreduce': variance_mapreduce,
        'std_deviation': std_deviation,
        'min_price': np.min(prices),
        'max_price': np.max(prices),
        'median_price': np.median(prices)
    }
    
    return statistics

def verify_mapreduce_formula(statistics):
    """
    Verify that the MapReduce variance formula matches direct calculation.
    
    Args:
        statistics (dict): Calculated statistics
        
    Returns:
        bool: True if formulas match
    """
    variance_direct = statistics['variance']
    variance_mapreduce = statistics['variance_mapreduce']
    
    # Check if the two variance calculations match (within floating point tolerance)
    formulas_match = np.isclose(variance_direct, variance_mapreduce, rtol=1e-10)
    
    print("\n" + "="*60)
    print("MAPREDUCE FORMULA VERIFICATION")
    print("="*60)
    print(f"Direct variance calculation: {variance_direct:.6f}")
    print(f"MapReduce formula (E[X²] - (E[X])²): {variance_mapreduce:.6f}")
    print(f"Formulas match: {formulas_match}")
    
    if not formulas_match:
        print("WARNING: Variance calculations don't match!")
        difference = abs(variance_direct - variance_mapreduce)
        print(f"Difference: {difference:.10f}")
    
    return formulas_match

def print_detailed_statistics(statistics):
    """
    Print detailed statistics in a formatted way.
    
    Args:
        statistics (dict): Calculated statistics
    """
    print("\n" + "="*60)
    print("DETAILED STATISTICS - PYTHON CALCULATION")
    print("="*60)
    print(f"Count of records: {statistics['count']:,}")
    print(f"Sum of prices: {statistics['sum_prices']:,.2f}")
    print(f"Sum of squares: {statistics['sum_squares']:,.2f}")
    print(f"Mean price: {statistics['mean_price']:.2f}")
    print(f"Variance: {statistics['variance']:.2f}")
    print(f"Standard deviation: {statistics['std_deviation']:.2f}")
    print(f"Minimum price: {statistics['min_price']:.2f}")
    print(f"Maximum price: {statistics['max_price']:.2f}")
    print(f"Median price: {statistics['median_price']:.2f}")

def compare_with_mapreduce_output(statistics, mapreduce_output_dir="."):
    """
    Compare Python calculations with MapReduce output if available.
    
    Args:
        statistics (dict): Python calculated statistics
        mapreduce_output_dir (str): Directory containing MapReduce output files
    """
    output_mean_file = Path(mapreduce_output_dir) / "output_mean.txt"
    output_variance_file = Path(mapreduce_output_dir) / "output_variance.txt"
    
    python_mean = statistics['mean_price']
    python_variance = statistics['variance']
    python_std = statistics['std_deviation']
    
    print("\n" + "="*60)
    print("COMPARISON WITH MAPREDUCE OUTPUT")
    print("="*60)
    
    # Read MapReduce mean output
    if output_mean_file.exists():
        with open(output_mean_file, 'r') as f:
            mapreduce_mean_line = f.readline().strip()
            if mapreduce_mean_line:
                try:
                    _, mapreduce_mean_str = mapreduce_mean_line.split('\t')
                    mapreduce_mean = float(mapreduce_mean_str)
                    mean_diff = abs(python_mean - mapreduce_mean)
                    mean_match = np.isclose(python_mean, mapreduce_mean, rtol=1e-5)
                    
                    print(f"Python mean:    {python_mean:.6f}")
                    print(f"MapReduce mean: {mapreduce_mean:.6f}")
                    print(f"Mean difference: {mean_diff:.6f}")
                    print(f"Mean matches: {mean_match}")
                    
                except (ValueError, IndexError) as e:
                    print(f"Error parsing MapReduce mean output: {e}")
    else:
        print("MapReduce mean output file not found")
    
    # Read MapReduce variance output
    if output_variance_file.exists():
        with open(output_variance_file, 'r') as f:
            mapreduce_variance = None
            mapreduce_std = None
            
            for line in f:
                line = line.strip()
                if line.startswith('variance\t'):
                    try:
                        _, mapreduce_variance_str = line.split('\t')
                        mapreduce_variance = float(mapreduce_variance_str)
                    except (ValueError, IndexError) as e:
                        print(f"Error parsing MapReduce variance: {e}")
                
                elif line.startswith('std_deviation\t'):
                    try:
                        _, mapreduce_std_str = line.split('\t')
                        mapreduce_std = float(mapreduce_std_str)
                    except (ValueError, IndexError) as e:
                        print(f"Error parsing MapReduce std deviation: {e}")
            
            if mapreduce_variance is not None:
                variance_diff = abs(python_variance - mapreduce_variance)
                variance_match = np.isclose(python_variance, mapreduce_variance, rtol=1e-5)
                
                print(f"\nPython variance:    {python_variance:.6f}")
                print(f"MapReduce variance: {mapreduce_variance:.6f}")
                print(f"Variance difference: {variance_diff:.6f}")
                print(f"Variance matches: {variance_match}")
            
            if mapreduce_std is not None:
                std_diff = abs(python_std - mapreduce_std)
                std_match = np.isclose(python_std, mapreduce_std, rtol=1e-5)
                
                print(f"\nPython std dev:    {python_std:.6f}")
                print(f"MapReduce std dev: {mapreduce_std:.6f}")
                print(f"Std dev difference: {std_diff:.6f}")
                print(f"Std dev matches: {std_match}")
    else:
        print("MapReduce variance output file not found")

def analyze_price_distribution(prices):
    """
    Analyze the distribution of prices.
    
    Args:
        prices (numpy.array): Array of prices
    """
    print("\n" + "="*60)
    print("PRICE DISTRIBUTION ANALYSIS")
    print("="*60)
    
    # Price ranges
    ranges = [0, 50, 100, 150, 200, 300, 500, 1000, float('inf')]
    range_labels = [
        "$0-50", "$50-100", "$100-150", "$150-200", 
        "$200-300", "$300-500", "$500-1000", "$1000+"
    ]
    
    print("Price distribution:")
    for i in range(len(ranges) - 1):
        lower = ranges[i]
        upper = ranges[i + 1]
        if upper == float('inf'):
            count = np.sum(prices >= lower)
        else:
            count = np.sum((prices >= lower) & (prices < upper))
        
        percentage = (count / len(prices)) * 100
        print(f"  {range_labels[i]}: {count:4d} records ({percentage:5.1f}%)")
    
    # Top 10 most common prices
    unique_prices, counts = np.unique(prices, return_counts=True)
    top_indices = np.argsort(counts)[-10:][::-1]
    
    print(f"\nTop 10 most common prices:")
    for i, idx in enumerate(top_indices, 1):
        print(f"  {i:2d}. ${unique_prices[idx]:5.0f}: {counts[idx]:3d} occurrences")

def main():
    """Main function to run the verification."""
    print("="*70)
    print("MAPREDUCE NYC REAL ESTATE - CALCULATION VERIFICATION")
    print("="*70)
    
    # Determine input file path
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    input_file = project_root / "input" / "AB_NYC_2019.csv"
    
    # Alternative path for when running from build directory
    if not input_file.exists():
        input_file = Path("input") / "AB_NYC_2019.csv"
    
    if not input_file.exists():
        input_file = Path("..") / "input" / "AB_NYC_2019.csv"
    
    # Load and validate data
    df = load_and_validate_data(input_file)
    
    # Extract prices
    prices = extract_prices(df)
    
    # Calculate statistics
    statistics = calculate_statistics(prices)
    
    # Print results
    print_detailed_statistics(statistics)
    
    # Verify MapReduce formula
    verify_mapreduce_formula(statistics)
    
    # Analyze price distribution
    analyze_price_distribution(prices)
    
    # Compare with MapReduce output if available
    compare_with_mapreduce_output(statistics)
    
    print("\n" + "="*70)
    print("VERIFICATION COMPLETED SUCCESSFULLY")
    print("="*70)

if __name__ == "__main__":
    main()
    