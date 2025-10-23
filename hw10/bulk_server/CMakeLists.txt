cmake_minimum_required(VERSION 3.15)
set(PATCH_VERSION "10" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.10.${PATCH_VERSION})
project(bulk_server VERSION ${PROJECT_VERSION} 
        DESCRIPTION "Bulk commands processing server with async I/O"
        HOMEPAGE_URL "https://github.com/godzilloff/otus-cpp-profession/tree/hw10"
        LANGUAGES CXX)

# Настройки стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Политики CMake
cmake_policy(SET CMP0144 NEW)  # Для FindPython
cmake_policy(SET CMP0167 NEW)  # Для поддержки новых версий

# Настройка путей Boost
if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT $ENV{BOOST_ROOT} CACHE PATH "Boost root directory")
endif()

if(DEFINED ENV{BOOST_LIBRARYDIR})
    set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR} CACHE PATH "Boost libraries directory")
endif()

# === BOOST ===
find_package(Boost REQUIRED COMPONENTS 
    system
    # filesystem, program_options, iostreams - удаляем, если не используются
)

if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
else()
    message(FATAL_ERROR "Boost not found! Please set BOOST_ROOT environment variable.")
endif()

# === Google Test ===
# Условная загрузка GTest
if(BUILD_TESTING)
    if(MSVC AND NOT GTEST_FOUND)
        set(GTEST_ROOT "c:/lib/googletest/" CACHE PATH "Path to Google Test")
    endif()
    
    find_package(GTest)
    if(GTest_FOUND)
        message(STATUS "GTest found: ${GTest_VERSION}")
        include(GoogleTest)
    else()
        message(WARNING "GTest not found - tests will be disabled")
    endif()
endif()

# === Библиотека async ===
add_library(async SHARED 
    src/async.cpp 
    src/command_processor.cpp
    src/thread_pool.cpp
    #src/observers/console_observer.h
    #src/observers/file_observer.h
)

if(WIN32)
    target_compile_definitions(async PRIVATE ASYNC_EXPORTS)
endif()

target_include_directories(async PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Современный способ линковки Boost
target_link_libraries(async PUBLIC Boost::boost)  # Заголовочные зависимости
target_link_libraries(async PRIVATE Boost::system) # Библиотечные зависимости

# Настройки для Windows
if(WIN32)
    target_compile_definitions(async PRIVATE ASYNC_EXPORTS)
    # Отключаем предупреждения для Boost и настраиваем Windows target
    target_compile_definitions(async PRIVATE _WIN32_WINNT=0x0601)
    target_compile_definitions(async PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Настройки именования библиотеки
set_target_properties(async PROPERTIES
    OUTPUT_NAME "async"
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
)

if(NOT WIN32)
    set_target_properties(async PROPERTIES PREFIX "lib")
endif()

# === Сервер bulk_server ===
add_executable(bulk_server 
    app/server/main.cpp
    app/server/async_server.cpp
    app/server/session.cpp
)

target_include_directories(bulk_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Линкуем библиотеку async и Boost
target_link_libraries(bulk_server PRIVATE async Boost::system)

message(STATUS "Linking bulk_server with: ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/async.lib")

if(WIN32)
    target_compile_definitions(bulk_server PRIVATE _WIN32_WINNT=0x0601)
endif()

# === Клиент bulk_client ===
add_executable(bulk_client 
    app/client/main.cpp
)

target_include_directories(bulk_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(bulk_client PRIVATE Boost::system)

if(WIN32)
    target_compile_definitions(bulk_client PRIVATE _WIN32_WINNT=0x0601)
endif()

# === Настройки компилятора ===
# Универсальные настройки предупреждений
if(MSVC)
    target_compile_options(async PRIVATE /W4 /permissive-)
    target_compile_options(bulk_server PRIVATE /W4 /permissive-)
    target_compile_options(bulk_client PRIVATE /W4 /permissive-)
else()
    target_compile_options(async PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(bulk_server PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(bulk_client PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# === Настройки для Visual Studio ===
if(MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    
    # Группировка проектов в Solution Explorer
    set_target_properties(async PROPERTIES FOLDER "Libraries")
    set_target_properties(bulk_server PROPERTIES FOLDER "Applications")
    set_target_properties(bulk_client PROPERTIES FOLDER "Applications")
    
    # Установка стартового проекта
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                 PROPERTY VS_STARTUP_PROJECT bulk_server)
endif()

# === Установка ===
install(TARGETS async bulk_server bulk_client
    EXPORT bulk_server-targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

# Установка заголовочных файлов
install(DIRECTORY include/ 
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Создание файла конфигурации для find_package()
install(EXPORT bulk_server-targets
    FILE bulk_server-config.cmake
    NAMESPACE bulk_server::
    DESTINATION lib/cmake/bulk_server
)

# === CPack настройки ===
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "bulk_server")
set(CPACK_PACKAGE_VENDOR "Otus")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Bulk commands processing server")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "10")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_CONTACT "test@gmail.com")
set(CPACK_DEBIAN_PACKAGE_NAME "bulk_server")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Test User <test@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system1.74 (>= 1.74.0)")
set(CPACK_DEBIAN_FILE_NAME "bulk_server-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.deb")

include(CPack)

# === Дополнительные цели ===
# Цель для запуска тестов
if(GTest_FOUND AND TARGET bulk_server)
    add_test(NAME bulk_server_test COMMAND bulk_server 9000 3)
endif()

# Цель для отладочной информации
add_custom_target(info
    COMMAND ${CMAKE_COMMAND} -E echo "Project: ${PROJECT_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo "Version: ${PROJECT_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Boost: ${Boost_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Compiler: ${CMAKE_CXX_COMPILER_ID}"
    COMMENT "Project information"
)
