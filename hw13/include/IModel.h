/**
 * @file IModel.h
 * @brief Интерфейс для моделей машинного обучения
 */

#pragma once

#include <vector>
#include <string>

/**
 * @brief Интерфейс для моделей классификации
 */
class IModel {
public:
    virtual ~IModel() = default;

    /**
     * @brief Загружает модель из файлов
     * @param model_paths Пути к файлам модели
     * @return true если загрузка успешна, false otherwise
     */
    virtual bool loadModel(const std::vector<std::string>& model_paths) = 0;

    /**
     * @brief Предсказывает класс для одного изображения
     * @param image Вектор пикселей изображения (784 элемента)
     * @return Предсказанный класс (0-9)
     */
    virtual int predict(const std::vector<float>& image) = 0;

    /**
     * @brief Предсказывает классы для набора изображений
     * @param images Вектор изображений
     * @return Вектор предсказанных классов
     */
    virtual std::vector<int> predictBatch(const std::vector<std::vector<float>>& images) = 0;
};
