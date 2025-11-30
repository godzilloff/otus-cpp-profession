/**
 * @file main.cpp
 * @brief Главный файл приложения для классификации Fashion MNIST
 * @details Принимает два аргумента: test.csv и директорию с моделью
 * @example ./fashion_mnist input/test.csv input/
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>

#include "MLPModel.h"
#include "CSVReader.h"

/**
 * @brief Вычисляет accuracy модели
 * @param true_labels Истинные метки
 * @param predicted_labels Предсказанные метки
 * @return Accuracy (доля правильных ответов)
 */
double calculateAccuracy(const std::vector<int>& true_labels, 
                        const std::vector<int>& predicted_labels) {
    if (true_labels.size() != predicted_labels.size()) {
        throw std::invalid_argument("Labels sizes don't match");
    }

    size_t correct = 0;
    for (size_t i = 0; i < true_labels.size(); i++) {
        if (true_labels[i] == predicted_labels[i]) {
            correct++;
        }
    }

    return static_cast<double>(correct) / true_labels.size();
}

/**
 * @brief Точка входа в приложение
 * @param argc Количество аргументов
 * @param argv Аргументы командной строки
 * @return Код возврата: 0 - успех, 1 - ошибка
 */
int main(int argc, char* argv[]) {
    // Проверяем аргументы командной строки
    if (argc != 3) {        
        std::cerr << "\033[31m"
                  << "Usage: " << argv[0] << " <test_csv_file> <model_directory>" << std::endl
                  << "Example: " << argv[0] << " input/test.csv input/" << std::endl
                  << "\033[0m";
        return 1;
    }

    const std::string test_csv_path = argv[1];
    const std::string model_dir = argv[2];

    try {
        // 1. Читаем тестовые данные
        // std::cout << "Loading test data from: " << test_csv_path << std::endl;
        auto test_data = CSVReader::readCSV(test_csv_path);
        // std::cout << "Loaded " << test_data.size() << " test samples" << std::endl;

        // Подготавливаем данные для предсказания
        std::vector<std::vector<float>> images;
        std::vector<int> true_labels;
        
        for (const auto& sample : test_data) {
            images.push_back(sample.features);
            true_labels.push_back(sample.label);
        }

        // 2. Загружаем модель MLP
        // std::cout << "Loading MLP model from directory: " << model_dir << std::endl;
        auto model = std::make_unique<MLPModel>();
        
        std::vector<std::string> model_paths = {
            model_dir + "/w1.txt",
            model_dir + "/w2.txt"
        };
        
        if (!model->loadModel(model_paths)) {
            std::cerr << "Failed to load MLP model" << std::endl;
            return 1;
        }

        // 3. Выполняем предсказания
        // std::cout << "Running predictions..." << std::endl;
        auto predictions = model->predictBatch(images);

        // 4. Вычисляем accuracy
        double accuracy = calculateAccuracy(true_labels, predictions);
        
        // 5. Выводим результат с точностью до 3 знаков
        std::cout << std::fixed << std::setprecision(3) << accuracy << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
