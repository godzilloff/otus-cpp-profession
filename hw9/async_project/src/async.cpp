/**
 * @file async.cpp
 * @brief Реализация функций библиотеки асинхронной обработки команд
 * 
 * Этот файл содержит реализацию C-интерфейса для работы с обработчиками команд.
 * Обеспечивает управление множественными контекстами обработки с потокобезопасностью.
 */

#include "async.h"

#include "command_processor.h"
#include <map>
#include <mutex>
#include <string_view>

namespace async {

namespace {
    /**
     * @brief Глобальная карта для хранения обработчиков команд
     * 
     * Содержит все активные обработчики команд, ассоциированные с их контекстами.
     * Ключ: указатель-контекст, полученный из connect()
     * Значение: уникальный указатель на объект CommandProcessor
     */
    std::map<void*, std::unique_ptr<CommandProcessor>> processors;
    
    /**
     * @brief Мьютекс для защиты глобальной карты обработчиков
     * 
     * Обеспечивает потокобезопасный доступ к карте processors при одновременных
     * вызовах connect(), receive() и disconnect() из разных потоков.
     */
    std::mutex processors_mutex;
}

/**
 * @brief Создает новый контекст обработки команд
 * 
 * Инициализирует новый обработчик команд с указанным размером блока и регистрирует
 * его в глобальной карте обработчиков. Создает фоновые потоки для вывода.
 * 
 * @param bulk_size Размер блока команд для автоматической обработки
 * @return handle_t Указатель на созданный контекст обработчика
 * 
 * @note Создает 3 фоновых потока для каждого обработчика:
 *       - 1 поток для вывода в консоль
 *       - 2 потока для записи в файлы
 * 
 * @warning Необходимо вызывать disconnect() для освобождения ресурсов
 * @warning Потокобезопасна - защищена мьютексом processors_mutex
 * 
 * @code
 * // Пример: создание обработчика с блоком из 5 команд
 * handle_t h = connect(5);
 * @endcode
 */
handle_t connect(size_t bulk_size) {
    std::lock_guard<std::mutex> lock(processors_mutex);
    
    // Создаем новый обработчик команд с заданным размером блока
    auto processor = std::make_unique<CommandProcessor>(bulk_size);
    void* handle = processor.get();
    
    // Регистрируем обработчик в глобальной карте
    processors.emplace(handle, std::move(processor));
    return handle;
}

/**
 * @brief Передает команду на обработку в указанный контекст
 * 
 * Добавляет команду в буфер указанного обработчика. Когда накапливается 
 * достаточное количество команд (согласно размеру блока), они автоматически
 * обрабатываются и отправляются в фоновые потоки для вывода.
 * 
 * @param handle Контекст обработчика, полученный из connect()
 * @param data Указатель на буфер с данными команды
 * @param data_size Размер данных команды в байтах
 * 
 * @note Если handle невалиден (уже удален или не существует), команда игнорируется
 * @note Потокобезопасна - защищена мьютексом processors_mutex
 * @note Использует string_view для эффективной работы со строковыми данными
 * 
 * @code
 * // Пример: передача нескольких команд
 * receive(h, "start", 5);
 * receive(h, "process_data", 12);
 * receive(h, "end", 3);
 * @endcode
 */
void receive(handle_t handle, const char* data, size_t data_size) {
    std::lock_guard<std::mutex> lock(processors_mutex);
    
    // Ищем обработчик по контексту
    if (auto it = processors.find(handle); it != processors.end()) {
        // Передаем команду на обработку, используя string_view для избежания копирования
        it->second->process_command(std::string_view{data, data_size});
    }
    // Если обработчик не найден (невалидный handle), команда игнорируется
}

/**
 * @brief Завершает работу с контекстом обработчика и освобождает ресурсы
 * 
 * Выполняет принудительную обработку оставшихся команд, останавливает фоновые
 * потоки, освобождает ресурсы и удаляет обработчик из глобальной карты.
 * 
 * @param handle Контекст обработчика для завершения
 * 
 * @note После вызова disconnect() handle становится невалидным
 * @note Выполняет join для всех фоновых потоков обработчика
 * @note Если handle невалиден, функция завершается без действий
 * @note Потокобезопасна - защищена мьютексом processors_mutex
 * 
 * @code
 * // Пример: корректное завершение работы
 * disconnect(h);  // Ресурсы освобождены, h больше не валиден
 * // receive(h, "new_cmd", 7);  // ОШИБКА: h невалиден!
 * @endcode
 */
void disconnect(handle_t handle) {
    std::lock_guard<std::mutex> lock(processors_mutex);
    
    // Ищем обработчик по контексту
    if (auto it = processors.find(handle); it != processors.end()) {
        // Обрабатываем оставшиеся команды в буфере
        it->second->flush_remaining();
        
        // Останавливаем фоновые потоки
        it->second->shutdown();
        
        // Удаляем обработчик из глобальной карты
        // unique_ptr автоматически освобождает память
        processors.erase(it);
    }
    // Если обработчик не найден, ничего не делаем
}

} // namespace async
