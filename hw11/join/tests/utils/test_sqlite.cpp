#include <sqlite3.h>
#include <iostream>
#include <string>

// Callback функция для SQL запросов
static int callback(void* data, int argc, char** argv, char** azColName) {
    for(int i = 0; i < argc; i++) {
        std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
    }
    std::cout << "-------------------" << std::endl;
    return 0;
}

int main() {
    sqlite3* db;
    char* errMsg = nullptr;
    
    // Открыть базу данных
    int rc = sqlite3_open(":memory:", &db);
    
    if(rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }
    
    std::cout << "SQLite version: " << sqlite3_libversion() << std::endl;
    std::cout << "Database opened successfully!" << std::endl;
    
    // Создать таблицу
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS Users ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Name TEXT NOT NULL, "
        "Email TEXT NOT NULL);";
    
    rc = sqlite3_exec(db, create_table_sql, nullptr, nullptr, &errMsg);
    
    if(rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Table created successfully!" << std::endl;
    }
    
    // Вставить данные
    const char* insert_sql = 
        "INSERT INTO Users (Name, Email) VALUES "
        "('John Doe', 'john@example.com'), "
        "('Jane Smith', 'jane@example.com');";
    
    rc = sqlite3_exec(db, insert_sql, nullptr, nullptr, &errMsg);
    
    if(rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Data inserted successfully!" << std::endl;
    }
    
    // Выбрать данные
    const char* select_sql = "SELECT * FROM Users;";
    
    std::cout << "Query results:" << std::endl;
    rc = sqlite3_exec(db, select_sql, callback, nullptr, &errMsg);
    
    if(rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    // Закрыть базу данных
    sqlite3_close(db);
    
    return 0;
}
