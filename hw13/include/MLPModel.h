/**
 * @file MLPModel.h
 * @brief Реализация многослойного перцептрона для классификации Fashion MNIST
 */

#pragma once

#include "IModel.h"
#include "MatrixUtils.h"
#include "CSVReader.h"
#include <memory>
#include <vector>
#include <string>
#include <iostream>

/**
 * @brief Многослойный перцептрон с двумя скрытыми слоями
 */
class MLPModel : public IModel {
private:
    std::vector<std::vector<float>> w1_; ///< Веса первого слоя (784x128) - уже транспонированы
    std::vector<std::vector<float>> w2_; ///< Веса второго слоя (128x10) - уже транспонированы
    bool is_loaded_ = false;             ///< Флаг загрузки модели

public:
    MLPModel() = default;
    ~MLPModel() override = default;

    /**
     * @brief Загружает веса модели из файлов
     * @param model_paths Вектор путей к файлам [w1_path, w2_path]
     * @return true если загрузка успешна
     */
    bool loadModel(const std::vector<std::string>& model_paths) override;

    /**
     * @brief Предсказывает класс для одного изображения
     * @param image Вектор пикселей изображения (784 элемента)
     * @return Предсказанный класс (0-9)
     */
    int predict(const std::vector<float>& image) override;

    /**
     * @brief Предсказывает классы для набора изображений
     * @param images Вектор изображений
     * @return Вектор предсказанных классов
     */
    std::vector<int> predictBatch(const std::vector<std::vector<float>>& images) override;

    /**
     * @brief Проверяет загружена ли модель
     * @return true если модель загружена
     */
    bool isLoaded() const { return is_loaded_; }

    /**
     * @brief Выводит информацию о модели
     */
    void printModelInfo() const {
        if (!is_loaded_) {
            std::cout << "Model is not loaded" << std::endl;
            return;
        }
        // std::cout << "MLP Model Info:" << std::endl;
        // std::cout << "  Layer 1 weights: " << w1_.size() << " x " << (w1_.empty() ? 0 : w1_[0].size()) << std::endl;
        // std::cout << "  Layer 2 weights: " << w2_.size() << " x " << (w2_.empty() ? 0 : w2_[0].size()) << std::endl;
    }

private:
    /**
     * @brief Выполняет прямой проход через сеть
     * @param input Входное изображение
     * @return Вектор вероятностей для каждого класса
     */
    std::vector<float> forwardPass(const std::vector<float>& input) const;
};
