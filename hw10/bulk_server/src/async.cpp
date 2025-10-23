#include "async.h"
#include "command_processor.h"
#include "observers/console_observer.h"
#include "observers/file_observer.h"
#include <map>
#include <mutex>
#include <string_view>
#include <memory>

namespace async_lib {

namespace {
    /**
     * @brief Глобальная карта для хранения обработчиков команд
     * 
     * @details Содержит все активные обработчики команд, ассоциированные с их контекстами.
     * Ключ: указатель-контекст, возвращаемый функцией async_connect()
     * Значение: уникальный указатель на объект CommandProcessor
     * 
     * @note Доступ к карте защищен мьютексом processors_mutex
     */
    std::map<void*, std::unique_ptr<CommandProcessor>> processors;
    
    /**
     * @brief Мьютекс для защиты глобальной карты обработчиков
     * 
     * @details Обеспечивает потокобезопасный доступ к карте processors при одновременных
     * вызовах async_connect(), async_receive() и async_disconnect() из разных потоков.
     */
    std::mutex processors_mutex;
    
    // Глобальные наблюдатели по умолчанию
    
    /**
     * @brief Глобальный наблюдатель для вывода в консоль
     * 
     * @details Создается один раз при старте программы и используется всеми
     * обработчиками команд для вывода блоков в стандартный поток вывода.
     */
    std::shared_ptr<ConsoleObserver> console_observer = std::make_shared<ConsoleObserver>();
    
    /**
     * @brief Глобальный наблюдатель для записи в файлы
     * 
     * @details Создается один раз при старте программы и используется всеми
     * обработчиками команд для записи блоков в файлы с уникальными именами.
     */
    std::shared_ptr<FileObserver> file_observer = std::make_shared<FileObserver>();
}

/**
 * @brief Создает новый контекст для обработки команд
 * @param bulk_size Размер блока команд для автоматической обработки
 * @return handle_t Указатель на созданный контекст обработчика
 * 
 * @details Инициализирует новый обработчик команд с заданным размером блока и
 * автоматически подписывает на него стандартных наблюдателей (консоль и файл).
 * Регистрирует обработчик в глобальной карте для последующего доступа.
 * 
 * @note Каждый вызов создает независимый контекст обработки
 * @note Автоматически подписывает консольного и файлового наблюдателей
 * @warning Необходимо вызывать async_disconnect() для освобождения ресурсов
 * 
 * @code
 * // Пример использования
 * handle_t h = async_connect(3);  // Создает обработчик с блоком из 3 команд
 * @endcode
 */
handle_t async_connect(size_t bulk_size) {
    std::lock_guard<std::mutex> lock(processors_mutex);
    
    auto processor = std::make_unique<CommandProcessor>(bulk_size);
    void* handle = processor.get();
    
    // Автоматически подписываем стандартных наблюдателей
    processor->subscribe(console_observer);
    processor->subscribe(file_observer);
    
    processors.emplace(handle, std::move(processor));
    return handle;
}

/**
 * @brief Передает команду на обработку в указанный контекст
 * @param handle Контекст обработчика, полученный из async_connect()
 * @param data Указатель на буфер с данными команды
 * @param data_size Размер данных команды в байтах
 * 
 * @details Добавляет команду в буфер указанного обработчика. Когда накапливается 
 * достаточное количество команд (согласно размеру блока), они автоматически
 * обрабатываются и отправляются наблюдателям для вывода.
 * 
 * @note Если handle невалиден (уже удален или не существует), команда игнорируется
 * @note Потокобезопасна - защищена мьютексом processors_mutex
 * @note Использует string_view для эффективной работы со строковыми данными
 * 
 * @code
 * // Пример: передача нескольких команд
 * async_receive(h, "start", 5);
 * async_receive(h, "process_data", 12);
 * async_receive(h, "end", 3);
 * @endcode
 */
void async_receive(handle_t handle, const char* data, size_t data_size) {
    std::lock_guard<std::mutex> lock(processors_mutex);
    
    if (auto it = processors.find(handle); it != processors.end()) {
        it->second->process_command(std::string_view{data, data_size});
    }
}

/**
 * @brief Завершает работу с контекстом обработчика и освобождает ресурсы
 * @param handle Контекст обработчика для завершения
 * 
 * @details Выполняет принудительную обработку оставшихся команд, останавливает фоновые
 * потоки, освобождает ресурсы и удаляет обработчик из глобальной карты.
 * 
 * @note После вызова async_disconnect() handle становится невалидным
 * @note Выполняет flush_remaining() для обработки последних команд
 * @note Если handle невалиден, функция завершается без действий
 * @note Потокобезопасна - защищена мьютексом processors_mutex
 * 
 * @code
 * // Пример: корректное завершение работы
 * async_disconnect(h);  // Ресурсы освобождены, h больше не валиден
 * // async_receive(h, "new_cmd", 7);  // ОШИБКА: h невалиден!
 * @endcode
 */
void async_disconnect(handle_t handle) {
    std::lock_guard<std::mutex> lock(processors_mutex);
    
    if (auto it = processors.find(handle); it != processors.end()) {
        it->second->flush_remaining();
        it->second->shutdown();
        processors.erase(it);
    }
}

/**
 * @brief Подписывает пользовательского наблюдателя на контекст
 * @param handle Контекст обработчика
 * @param observer_handle Указатель на пользовательского наблюдателя
 * 
 * @details Добавляет пользовательского наблюдателя в список подписчиков
 * указанного обработчика команд. После подписки наблюдатель будет получать
 * уведомления о всех сформированных блоках команд данного обработчика.
 * 
 * @note Наблюдатель должен реализовывать интерфейс IObserver
 * @note Память для пользовательского наблюдателя управляется вызывающим кодом
 * @note Если handle невалиден, функция завершается без действий
 * 
 * @code
 * // Пример: подписка пользовательского наблюдателя
 * class MyObserver : public IObserver { ... };
 * MyObserver my_observer;
 * async_subscribe(h, &my_observer);
 * @endcode
 */
void async_subscribe(handle_t handle, void* observer_handle) {
    std::lock_guard<std::mutex> lock(processors_mutex);
    
    if (auto it = processors.find(handle); it != processors.end()) {
        auto observer = static_cast<IObserver*>(observer_handle);
        it->second->subscribe(std::shared_ptr<IObserver>(observer, [](IObserver*){}));
    }
}

/**
 * @brief Отписывает пользовательского наблюдателя от контекста
 * @param handle Контекст обработчика
 * @param observer_handle Указатель на пользовательского наблюдателя
 * 
 * @details Удаляет пользовательского наблюдателя из списка подписчиков
 * указанного обработчика команд. После отписки наблюдатель больше не будет
 * получать уведомления от данного обработчика.
 * 
 * @note Если наблюдатель не был подписан, функция завершается без действий
 * @note Если handle невалиден, функция завершается без действий
 * 
 * @code
 * // Пример: отписка пользовательского наблюдателя
 * async_unsubscribe(h, &my_observer);
 * @endcode
 */
void async_unsubscribe(handle_t handle, void* observer_handle) {
    std::lock_guard<std::mutex> lock(processors_mutex);
    
    if (auto it = processors.find(handle); it != processors.end()) {
        auto observer = static_cast<IObserver*>(observer_handle);
        it->second->unsubscribe(std::shared_ptr<IObserver>(observer, [](IObserver*){}));
    }
}

} // namespace async_lib
