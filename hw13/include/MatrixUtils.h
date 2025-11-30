/**
 * @file MatrixUtils.h
 * @brief Утилиты для работы с матрицами и математическими операциями
 */

#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>

/**
 * @brief Класс с математическими утилитами для ML
 */
class MatrixUtils {
public:
    /**
     * @brief Применяет сигмоидную функцию активации к вектору
     * @param x Входной вектор
     * @return Вектор после применения сигмоида
     */
    static std::vector<float> sigmoid(const std::vector<float>& x) {
        std::vector<float> result(x.size());
        for (size_t i = 0; i < x.size(); i++) {
            result[i] = 1.0f / (1.0f + std::exp(-x[i]));
        }
        return result;
    }

    /**
     * @brief Применяет функцию softmax к вектору
     * @param x Входной вектор
     * @return Вектор вероятностей (сумма = 1.0)
     */
    static std::vector<float> softmax(const std::vector<float>& x) {
        std::vector<float> result(x.size());
        float max_val = *std::max_element(x.begin(), x.end());
        float sum = 0.0f;
        
        // Стабильный softmax (вычитаем максимум)
        for (size_t i = 0; i < x.size(); i++) {
            result[i] = std::exp(x[i] - max_val);
            sum += result[i];
        }
        
        // Нормализация
        for (size_t i = 0; i < x.size(); i++) {
            result[i] /= sum;
        }
        return result;
    }

    /**
     * @brief Умножение матрицы на вектор
     * @param matrix Матрица размером M x N
     * @param vector Вектор размером N
     * @return Результат умножения размером M
     */
    static std::vector<float> matrixVectorMultiply(const std::vector<std::vector<float>>& matrix, 
                                                  const std::vector<float>& vector) {
        if (matrix.empty() || matrix[0].size() != vector.size()) {
            throw std::invalid_argument("Matrix and vector dimensions don't match");
        }

        std::vector<float> result(matrix.size(), 0.0f);
        for (size_t i = 0; i < matrix.size(); i++) {
            for (size_t j = 0; j < vector.size(); j++) {
                result[i] += matrix[i][j] * vector[j];
            }
        }
        return result;
    }

    /**
     * @brief Находит индекс максимального элемента в векторе
     * @param vec Входной вектор
     * @return Индекс максимального элемента
     */
    static size_t argmax(const std::vector<float>& vec) {
        return std::distance(vec.begin(), std::max_element(vec.begin(), vec.end()));
    }

    /**
     * @brief Нормализует пиксели изображения в диапазон [0, 1]
     * @param image Входное изображение
     * @return Нормализованное изображение
     */
    static std::vector<float> normalizeImage(const std::vector<float>& image) {
        std::vector<float> normalized(image.size());
        for (size_t i = 0; i < image.size(); i++) {
            normalized[i] = image[i] / 255.0f;
        }
        return normalized;
    }
};
