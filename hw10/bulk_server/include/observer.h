#pragma once

#include <vector>
#include <string>
#include <memory>
#include <ctime>

/**
 * @brief Интерфейс наблюдателя в паттерне "Наблюдатель"
 */
class IObserver {
public:
    virtual ~IObserver() = default;
    
    /**
     * @brief Обработка нового блока команд
     * @param commands Вектор команд в блоке
     * @param timestamp Временная метка создания блока
     * @param context_id Идентификатор контекста (соединения)
     */
    virtual void update(const std::vector<std::string>& commands, 
                       time_t timestamp, 
                       void* context_id) = 0;
};

/**
 * @brief Интерфейс наблюдаемого объекта
 */
class IObservable {
public:
    virtual ~IObservable() = default;
    
    /**
     * @brief Подписка наблюдателя
     * @param observer Указатель на наблюдателя
     */
    virtual void subscribe(std::shared_ptr<IObserver> observer) = 0;
    
    /**
     * @brief Отписка наблюдателя
     * @param observer Указатель на наблюдателя
     */
    virtual void unsubscribe(std::shared_ptr<IObserver> observer) = 0;
    
    /**
     * @brief Уведомление всех наблюдателей
     * @param commands Вектор команд
     * @param timestamp Временная метка
     * @param context_id Идентификатор контекста
     */
    virtual void notify(const std::vector<std::string>& commands, 
                       time_t timestamp, 
                       void* context_id) = 0;
};
