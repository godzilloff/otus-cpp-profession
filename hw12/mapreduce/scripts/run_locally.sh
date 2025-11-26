#!/bin/bash

echo "========================================"
echo "MapReduce NYC Real Estate Analysis"
echo "========================================"
echo ""

echo "[INFO] Checking required files..."
if [ ! -f "bin/mapper_mean" ]; then
    echo "[ERROR] mapper_mean not found!"
    exit 1
fi
if [ ! -f "bin/reducer_mean" ]; then
    echo "[ERROR] reducer_mean not found!"
    exit 1
fi
if [ ! -f "input/AB_NYC_2019.csv" ]; then
    echo "[ERROR] AB_NYC_2019.csv not found!"
    exit 1
fi

echo "[INFO] Calculating Mean Price..."
cat "input/AB_NYC_2019.csv" | ./bin/mapper_mean | sort -k1 | ./bin/reducer_mean > output_mean.txt
echo "Mean Price Results:"
cat output_mean.txt
echo ""

echo "[INFO] Calculating Variance..."
cat "input/AB_NYC_2019.csv" | ./bin/mapper_variance | sort -k1 | ./bin/reducer_variance > output_variance.txt
echo "Variance Results:"
cat output_variance.txt
echo ""

echo "[INFO] Running Python verification..."
python3 scripts/test_calculation.py

echo ""
echo "[INFO] All calculations completed!"
