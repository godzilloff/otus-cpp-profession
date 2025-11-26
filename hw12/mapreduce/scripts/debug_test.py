#!/usr/bin/env python3
"""
DEBUG TEST SCRIPT
"""

import os
import sys

print("=== PYTHON DEBUG TEST ===")
print(f"Python version: {sys.version}")
print(f"Current directory: {os.getcwd()}")

# List all files in current directory
print("\nFiles in current directory:")
for file in os.listdir('.'):
    print(f"  {file}")

# Check if data file exists
data_file = "AB_NYC_2019.csv"
print(f"\nChecking for data file: {data_file}")
if os.path.exists(data_file):
    print("✅ Data file FOUND!")
    print(f"File size: {os.path.getsize(data_file)} bytes")
    
    # Try to read first few lines
    try:
        with open(data_file, 'r', encoding='utf-8') as f:
            lines = []
            for i in range(5):
                line = f.readline().strip()
                if line:
                    lines.append(line)
            print("First 5 lines of file:")
            for i, line in enumerate(lines):
                print(f"  {i+1}: {line}")
    except Exception as e:
        print(f"Error reading file: {e}")
        
        # Try with different encoding
        try:
            with open(data_file, 'r', encoding='latin-1') as f:
                lines = []
                for i in range(5):
                    line = f.readline().strip()
                    if line:
                        lines.append(line)
                print("First 5 lines (latin-1 encoding):")
                for i, line in enumerate(lines):
                    print(f"  {i+1}: {line}")
        except Exception as e2:
            print(f"Error with latin-1 encoding: {e2}")
else:
    print("❌ Data file NOT FOUND!")

print("\n=== DEBUG COMPLETE ===")