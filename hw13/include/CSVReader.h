/**
 * @file CSVReader.h
 * @brief Класс для чтения CSV файлов с данными Fashion MNIST
 */

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

/**
 * @brief Структура для хранения одного примера данных
 */
struct DataSample {
    int label;                          ///< Истинная метка класса
    std::vector<float> features;        ///< Вектор признаков (пиксели)

    DataSample(int lbl, const std::vector<float>& feat) : label(lbl), features(feat) {}
};

/**
 * @brief Класс для чтения и парсинга CSV файлов
 */
class CSVReader {
public:
    /**
     * @brief Читает CSV файл с данными Fashion MNIST
     * @param filename Путь к CSV файлу
     * @return Вектор данных
     */
    static std::vector<DataSample> readCSV(const std::string& filename) {
        std::vector<DataSample> data;
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        size_t line_count = 0;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string cell;
            std::vector<float> features;
            int label = -1;
            int col_index = 0;

            // Первый столбец - метка, остальные - признаки
            while (std::getline(ss, cell, ',')) {
                if (col_index == 0) {
                    label = std::stoi(cell);
                } else {
                    features.push_back(static_cast<float>(std::stoi(cell)));
                }
                col_index++;
            }

            if (label == -1) {
                throw std::runtime_error("Invalid label in line: " + std::to_string(line_count));
            }

            if (features.size() != 784) {
                throw std::runtime_error("Invalid number of features in line " + 
                    std::to_string(line_count) + ": expected 784, got " + 
                    std::to_string(features.size()));
            }

            data.emplace_back(label, features);
            line_count++;
        }

        // std::cout << "Successfully read " << data.size() << " samples from " << filename << std::endl;
        return data;
    }

    /**
     * @brief Читает матрицу весов из текстового файла
     * @param filename Путь к файлу с весами
     * @param expected_rows Ожидаемое количество строк
     * @param expected_cols Ожидаемое количество столбцов
     * @return Матрица весов
     */
    static std::vector<std::vector<float>> readWeights(const std::string& filename, 
                                                      size_t expected_rows, 
                                                      size_t expected_cols) {
        std::vector<std::vector<float>> matrix;
        std::ifstream file(filename);

        if (!file.is_open()) {
            throw std::runtime_error("Cannot open weights file: " + filename);
        }

        std::string line;
        size_t row_count = 0;
        
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::vector<float> row;
            float value;
            
            while (ss >> value) {
                row.push_back(value);
            }
            
            if (!row.empty()) {
                if (row.size() != expected_cols) {
                    throw std::runtime_error("Invalid number of columns in row " + 
                        std::to_string(row_count) + " of " + filename + 
                        ": expected " + std::to_string(expected_cols) + 
                        ", got " + std::to_string(row.size()));
                }
                matrix.push_back(row);
                row_count++;
            }
        }

        if (matrix.size() != expected_rows) {
            throw std::runtime_error("Invalid number of rows in " + filename + 
                ": expected " + std::to_string(expected_rows) + 
                ", got " + std::to_string(matrix.size()));
        }

        // std::cout << "Successfully loaded weights from " << filename 
        //    << " with shape (" << matrix.size() << ", " << matrix[0].size() << ")" << std::endl;
        return matrix;
    }

    /**
     * @brief Транспонирует матрицу
     * @param matrix Исходная матрица
     * @return Транспонированная матрица
     */
    static std::vector<std::vector<float>> transpose(const std::vector<std::vector<float>>& matrix) {
        if (matrix.empty()) {
            return {};
        }

        size_t rows = matrix.size();
        size_t cols = matrix[0].size();
        
        std::vector<std::vector<float>> result(cols, std::vector<float>(rows));
        
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                result[j][i] = matrix[i][j];
            }
        }
        
        return result;
    }
};
