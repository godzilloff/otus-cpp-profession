#pragma once

/**
 * @file async_export.h
 * @brief Макросы для экспорта/импорта библиотеки
 * 
 * Этот файл содержит макросы для корректного экспорта функций библиотеки
 * на разных платформах (Windows/Linux).
 */

#ifdef _WIN32
    #ifdef ASYNC_EXPORTS
        /**
         * @brief Макрос для экспорта функций при сборке библиотеки
         */
        #define ASYNC_API __declspec(dllexport)
    #else
        /**
         * @brief Макрос для импорта функций при использовании библиотеки
         */
        #define ASYNC_API __declspec(dllimport)
    #endif
#else
    /**
     * @brief Пустой макрос для Unix-подобных систем
     */
    #define ASYNC_API
#endif
