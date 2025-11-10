/**
 * @file repository.cpp
 * @brief Реализация репозитория для работы с таблицами базы данных
 */

#include "repository.h"
#include <stdexcept>

/**
 * @brief Конструктор репозитория таблиц
 * @param db Указатель на объект базы данных
 */
TableRepository::TableRepository(std::shared_ptr<Database> db) 
    : db_(std::move(db)) {}

/**
 * @brief Вставка новой записи в указанную таблицу
 * @param table Название таблицы ("A" или "B")
 * @param id Уникальный идентификатор записи (первичный ключ)
 * @param name Значение имени для вставки
 * @return true если вставка прошла успешно, false в случае ошибки или дублирования ID
 * 
 * @throws нет явных исключений, но может возвращать false при ошибках SQLite
 */
bool TableRepository::insert(const std::string& table, int id, const std::string& name) {
    // Проверяем валидность имени таблицы
    if (table != "A" && table != "B") {
        return false;
    }
    
    // Формируем SQL запрос для вставки
    std::string sql = "INSERT INTO " + table + " (id, name) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    
    // Подготавливаем SQL запрос
    if (sqlite3_prepare_v2(db_->get_handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    // Привязываем параметры к запросу
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);

    // Выполняем запрос и проверяем результат
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    
    // Освобождаем ресурсы запроса
    sqlite3_finalize(stmt);

    return result;
}

/**
 * @brief Очистка указанной таблицы (удаление всех записей)
 * @param table Название таблицы ("A" или "B")
 * @return true если очистка прошла успешно, false в случае ошибки
 * 
 * @note Таблица остается существовать, но становится пустой
 */
bool TableRepository::truncate(const std::string& table) {
    // Проверяем валидность имени таблицы
    if (table != "A" && table != "B") {
        return false;
    }
    
    // Формируем SQL запрос для очистки таблицы
    std::string sql = "DELETE FROM " + table + ";";
    char* errmsg = nullptr;
    
    // Выполняем SQL запрос
    if (sqlite3_exec(db_->get_handle(), sql.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
        // Освобождаем память сообщения об ошибке
        sqlite3_free(errmsg);
        return false;
    }
    
    return true;
}

/**
 * @brief Выполнение операции пересечения таблиц A и B
 * @return Вектор строк результата пересечения, содержащий ID и значения из обеих таблиц
 * 
 * @details Возвращает записи, которые присутствуют в обеих таблицах с одинаковым ID.
 * Результат сортируется по возрастанию ID.
 * 
 * @see ResultRow
 */
std::vector<ResultRow> TableRepository::intersection() {
    // SQL запрос для получения пересечения таблиц
    std::string sql = R"(
        SELECT A.id, A.name, B.name 
        FROM A INNER JOIN B ON A.id = B.id
        ORDER BY A.id;
    )";
    
    return execute_join_query(sql);
}

/**
 * @brief Выполнение операции симметрической разности таблиц A и B
 * @return Вектор строк результата симметрической разности
 * 
 * @details Возвращает записи, которые присутствуют только в одной из таблиц.
 * Для записей из таблицы A поле column_b будет пустым, для таблицы B - column_a.
 * Результат сортируется по возрастанию ID.
 * 
 * @note В SQLite FULL OUTER JOIN эмулируется через комбинацию LEFT и RIGHT JOIN
 */
std::vector<ResultRow> TableRepository::symmetric_difference() {
    // SQL запрос для получения симметрической разности
    std::string sql = R"(
        SELECT 
            COALESCE(A.id, B.id) as id,
            A.name, 
            B.name
        FROM 
            A FULL OUTER JOIN B ON A.id = B.id
        WHERE 
            A.id IS NULL OR B.id IS NULL
        ORDER BY id;
    )";
    
    return execute_join_query(sql);
}

/**
 * @brief Вспомогательный метод для выполнения JOIN запросов
 * @param sql SQL запрос для выполнения
 * @return Вектор строк результата запроса
 * 
 * @details Выполняет подготовленный SQL запрос, обрабатывает результаты
 * и преобразует их в структуру ResultRow. Корректно обрабатывает NULL значения.
 * 
 * @warning Возвращает пустой вектор в случае ошибки выполнения запроса
 */
std::vector<ResultRow> TableRepository::execute_join_query(const std::string& sql) {
    sqlite3_stmt* stmt = nullptr;
    std::vector<ResultRow> results;
    
    // Подготавливаем SQL запрос
    if (sqlite3_prepare_v2(db_->get_handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return results;
    }

    // Обрабатываем каждую строку результата
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ResultRow row;
        
        // Извлекаем ID (всегда присутствует)
        row.id = sqlite3_column_int(stmt, 0);
        
        // Извлекаем имя из таблицы A (может быть NULL)
        const unsigned char* a_name = sqlite3_column_text(stmt, 1);
        row.a = a_name ? reinterpret_cast<const char*>(a_name) : "";
        
        // Извлекаем имя из таблицы B (может быть NULL)
        const unsigned char* b_name = sqlite3_column_text(stmt, 2);
        row.b = b_name ? reinterpret_cast<const char*>(b_name) : "";
        
        // Добавляем строку в результат
        results.push_back(row);
    }

    // Освобождаем ресурсы запроса
    sqlite3_finalize(stmt);
    return results;
}
