/**
 * @file MLPModel.cpp
 * @brief Реализация методов MLPModel
 */

#include "MLPModel.h"
#include <stdexcept>

bool MLPModel::loadModel(const std::vector<std::string>& model_paths) {
    if (model_paths.size() < 2) {
        throw std::invalid_argument("MLPModel requires exactly 2 weight files");
    }

    try {
        // std::cout << "Loading weights from:" << std::endl;
        // std::cout << "  w1: " << model_paths[0] << std::endl;
        // std::cout << "  w2: " << model_paths[1] << std::endl;

        // Загружаем веса в исходном формате
        auto w1_original = CSVReader::readWeights(model_paths[0], 784, 128);  // w1.txt: 784x128
        auto w2_original = CSVReader::readWeights(model_paths[1], 128, 10);   // w2.txt: 128x10
        
        // Транспонируем веса для эффективного умножения
        w1_ = CSVReader::transpose(w1_original);  // Теперь 128x784
        w2_ = CSVReader::transpose(w2_original);  // Теперь 10x128
        
        // std::cout << "Weights after transposition:" << std::endl;
        // std::cout << "  w1: " << w1_.size() << " x " << (w1_.empty() ? 0 : w1_[0].size()) << std::endl;
        // std::cout << "  w2: " << w2_.size() << " x " << (w2_.empty() ? 0 : w2_[0].size()) << std::endl;
        
        is_loaded_ = true;
        // std::cout << "MLP model successfully loaded" << std::endl;
        return true;
    } catch (const std::exception& e) {
        is_loaded_ = false;
        std::cerr << "Failed to load MLP model: " << e.what() << std::endl;
        return false;
    }
}

int MLPModel::predict(const std::vector<float>& image) {
    if (!is_loaded_) {
        throw std::runtime_error("Model is not loaded");
    }
    if (image.size() != 784) {
        throw std::invalid_argument("Input image must have exactly 784 features, got " + 
                                   std::to_string(image.size()));
    }

    try {
        auto probabilities = forwardPass(image);
        int prediction = static_cast<int>(MatrixUtils::argmax(probabilities));
        
        // Отладочная информация
        /*
        std::cout << "Prediction probabilities: ";
        for (size_t i = 0; i < probabilities.size(); i++) {
            std::cout << "[" << i << "]:" << probabilities[i] << " ";
        }
        std::cout << "-> predicted: " << prediction << std::endl;
        */
        
        return prediction;
    } catch (const std::exception& e) {
        throw std::runtime_error("Prediction failed: " + std::string(e.what()));
    }
}

std::vector<int> MLPModel::predictBatch(const std::vector<std::vector<float>>& images) {
    std::vector<int> predictions;
    predictions.reserve(images.size());

    size_t processed = 0;
    for (const auto& image : images) {
        predictions.push_back(predict(image));
        processed++;
        
        // Прогресс
        // if (processed % 1000 == 0) {
        //     std::cout << "Processed " << processed << "/" << images.size() << " images" << std::endl;
        // }
    }

    return predictions;
}

std::vector<float> MLPModel::forwardPass(const std::vector<float>& input) const {
    // Нормализация входных данных
    auto normalized = MatrixUtils::normalizeImage(input);
    
    // Первый слой: 784 -> 128 с сигмоидной активацией
    // w1_: 128x784, normalized: 784 -> результат: 128
    auto layer1_output = MatrixUtils::matrixVectorMultiply(w1_, normalized);
    auto layer1_activated = MatrixUtils::sigmoid(layer1_output);
    
    // Второй слой: 128 -> 10 с softmax активацией
    // w2_: 10x128, layer1_activated: 128 -> результат: 10
    auto layer2_output = MatrixUtils::matrixVectorMultiply(w2_, layer1_activated);
    return MatrixUtils::softmax(layer2_output);
}
