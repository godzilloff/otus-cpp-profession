#pragma once

#include "async_export.h"
#include <cstddef>

/**
 * @brief Пространство имен библиотеки асинхронной обработки команд
 */
namespace async_lib {

/**
 * @brief Тип для работы с контекстом обработчика команд
 */
using handle_t = void *;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Создает новый контекст для обработки команд
 * @param bulk_size Размер блока команд для автоматической обработки
 * @return handle_t Указатель на созданный контекст обработчика
 */
ASYNC_API handle_t async_connect(size_t bulk_size);

/**
 * @brief Передает команду на обработку в указанный контекст
 * @param handle Контекст обработчика, полученный из async_connect()
 * @param data   Указатель на буфер с данными команды
 * @param data_size Размер данных команды в байтах
 */
ASYNC_API void async_receive(handle_t handle, const char* data, size_t data_size);

/**
 * @brief Завершает работу с контекстом обработчика
 * @param handle Контекст обработчика для завершения
 */
ASYNC_API void async_disconnect(handle_t handle);

/**
 * @brief Подписывает наблюдателя на контекст
 * @param handle Контекст обработчика
 * @param observer_handle Указатель на наблюдателя
 */
ASYNC_API void async_subscribe(handle_t handle, void* observer_handle);

/**
 * @brief Отписывает наблюдателя от контекста
 * @param handle Контекст обработчика
 * @param observer_handle Указатель на наблюдателя
 */
ASYNC_API void async_unsubscribe(handle_t handle, void* observer_handle);

#ifdef __cplusplus
}
#endif

} // namespace async_lib
