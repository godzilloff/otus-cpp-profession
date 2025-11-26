@echo off
setlocal enabledelayedexpansion

echo ========================================
echo MapReduce NYC Real Estate Analysis
echo ========================================
echo.

echo [INFO] Checking required files...
if not exist "bin\mapper_mean.exe" (
    echo [ERROR] mapper_mean.exe not found!
    exit /b 1
)
if not exist "bin\reducer_mean.exe" (
    echo [ERROR] reducer_mean.exe not found!
    exit /b 1
)
if not exist "input\AB_NYC_2019.csv" (
    echo [ERROR] AB_NYC_2019.csv not found!
    exit /b 1
)

echo [INFO] Calculating Mean Price...
type "input\AB_NYC_2019.csv" | "bin\mapper_mean.exe" | sort | "bin\reducer_mean.exe" > output_mean.txt
echo Mean Price Results:
type output_mean.txt
echo.

echo [INFO] Calculating Variance...
type "input\AB_NYC_2019.csv" | "bin\mapper_variance.exe" | sort | "bin\reducer_variance.exe" > output_variance.txt
echo Variance Results:
type output_variance.txt
echo.

echo [INFO] Running Python verification...
python "scripts\test_calculation.py"

echo.
echo [INFO] All calculations completed!
endlocal
