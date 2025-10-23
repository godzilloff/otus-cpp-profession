#pragma once

#include "observer.h"
#include <fstream>
#include <atomic>
#include <chrono>

/**
 * @class FileObserver
 * @brief Наблюдатель для записи блоков команд в файлы
 * 
 * @details Реализует интерфейс IObserver для записи сформированных блоков команд
 * в отдельные файлы. Каждый файл получает уникальное имя на основе временной метки
 * и счетчика, что гарантирует отсутствие конфликтов имен при параллельной работе.
 * 
 * @note Имена файлов формируются по шаблону: bulk_<timestamp>_<microseconds>_<counter>.log
 * @note Использует атомарный счетчик для гарантии уникальности имен в многопоточной среде
 * @see IObserver
 */
class FileObserver : public IObserver {
public:
    /**
     * @brief Обрабатывает новый блок команд, записывая его в файл
     * @param commands Вектор команд в блоке
     * @param timestamp Временная метка создания блока (в секундах)
     * @param context_id Идентификатор контекста (не используется)
     * 
     * @details Создает файл с уникальным именем и записывает в него все команды
     * из блока в формате "bulk: cmd1, cmd2, cmd3". Для генерации уникального имени
     * используется комбинация временной метки, микросекунд и атомарного счетчика.
     * 
     * @note Если вектор команд пуст, метод завершается без действий
     * @note Игнорирует параметр context_id, так как файлы создаются независимо от контекста
     * @note Гарантирует потокобезопасность за счет использования атомарного счетчика
     * 
     * @code
     * // Пример создаваемого файла: bulk_1517223860_1517223860123456_5.log
     * // Содержимое: bulk: cmd1, cmd2, cmd3
     * @endcode
     */
    void update(const std::vector<std::string>& commands, 
                time_t timestamp, 
                void* context_id) override {
        (void)context_id; // Не используется в файловом наблюдателе
        
        if (commands.empty()) return;
        
        // Генерируем уникальное имя файла с высокой точностью
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        auto epoch = now_ms.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
        
        /**
         * @brief Атомарный счетчик файлов
         * 
         * @details Статический атомарный счетчик гарантирует уникальность имен
         * файлов даже при параллельных вызовах update() из разных потоков.
         * Счетчик увеличивается на каждый создаваемый файл.
         */
        static std::atomic<size_t> file_counter{0};
        size_t counter = file_counter++;
        
        /**
         * @brief Формирование имени файла
         * 
         * @details Имя файла состоит из:
         * - Префикса "bulk_"
         * - Временной метки в секундах (timestamp)
         * - Текущего времени в микросекундах (value.count())
         * - Порядкового номера файла (counter)
         * - Расширения ".log"
         * 
         * Такая комбинация гарантирует уникальность имени даже при высокой
         * частоте создания файлов в многопоточной среде.
         */
        std::string filename = "bulk_" + 
                              std::to_string(timestamp) + "_" +
                              std::to_string(value.count()) + "_" +
                              std::to_string(counter) + ".log";
        
        /**
         * @brief Запись блока команд в файл
         * 
         * @details Открывает файл для записи и записывает все команды из блока
         * в формате "bulk: cmd1, cmd2, cmd3". Если файл не может быть открыт,
         * запись не производится.
         * 
         * @note Использует RAII для управления ресурсами файла
         * @note Форматирование вывода соответствует требованиям задания
         */
        if (std::ofstream file{filename}) {
            file << "bulk: ";
            for (size_t i = 0; i < commands.size(); ++i) {
                if (i != 0) file << ", ";
                file << commands[i];
            }
            file << std::endl;
        }
    }
};
