cmake_minimum_required(VERSION 3.15)
set(PATCH_VERSION "11" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.11.${PATCH_VERSION})
project(join VERSION ${PROJECT_VERSION} 
        DESCRIPTION "Outer join"
        HOMEPAGE_URL "https://github.com/godzilloff/otus-cpp-profession/tree/hw11"
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
    # filesystem, program_options, iostreams
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

# === Библиотека SQLite ===
# Опция для выбора типа сборки SQLite
option(BUILD_STATIC_SQLITE "Build SQLite as static library" ON)
option(USE_SYSTEM_SQLITE "Use system SQLite if available" ON)

# Для Windows используем прекомпиленные библиотеки
# Для Linux/macOS компилируем из исходников

if(WIN32 AND NOT BUILD_STATIC_SQLITE)
    # Windows: используем прекомпиленные DLL
    message(STATUS "Using precompiled SQLite3 DLL for Windows")
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SQLITE3_LIB_DIR ${CMAKE_SOURCE_DIR}/third_party/sqlite3/lib/windows/x64)
    else()
        set(SQLITE3_LIB_DIR ${CMAKE_SOURCE_DIR}/third_party/sqlite3/lib/windows/x86)
    endif()
    
    set(SQLITE3_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/sqlite3/include)
    set(SQLITE3_LIBRARY ${SQLITE3_LIB_DIR}/sqlite3.lib)
    
    add_library(sqlite3 SHARED IMPORTED)
    set_target_properties(sqlite3 PROPERTIES
        IMPORTED_IMPLIB ${SQLITE3_LIBRARY}
        IMPORTED_LOCATION ${SQLITE3_LIB_DIR}/sqlite3.dll
        INTERFACE_INCLUDE_DIRECTORIES ${SQLITE3_INCLUDE_DIR}
    )
    
    # Копируем DLL в выходную директорию
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${SQLITE3_LIB_DIR}/sqlite3.dll
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    
else()
    # Linux/macOS или статическая сборка на Windows: компилируем из исходников
    if(USE_SYSTEM_SQLITE)
        # Пытаемся найти системную библиотеку
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(SQLITE3 QUIET sqlite3)
        endif()
        
        if(SQLITE3_FOUND)
            message(STATUS "Using system SQLite3 library")
            add_library(sqlite3_system INTERFACE IMPORTED)
            set_target_properties(sqlite3_system PROPERTIES
                INTERFACE_LINK_LIBRARIES ${SQLITE3_LIBRARIES}
                INTERFACE_INCLUDE_DIRECTORIES ${SQLITE3_INCLUDE_DIRS}
                INTERFACE_COMPILE_OPTIONS "${SQLITE3_CFLAGS_OTHER}"
            )
            add_library(sqlite3 ALIAS sqlite3_system)
        else()
            message(STATUS "System SQLite3 not found, building from source")
            add_subdirectory(third_party/sqlite3)
        endif()
    else()
        message(STATUS "Building SQLite3 from source")
        add_subdirectory(third_party/sqlite3)
    endif()
endif()


# === Сервер join_server ===
add_executable(join_server 
    app/server/main.cpp
    app/server/commands/command.cpp
    app/server/commands/command_factory.cpp
    app/server/database/database.cpp
    app/server/database/repository.cpp
    app/server/network/command_handler.cpp
    app/server/network/server.cpp
    app/server/network/session.cpp
)

# === Клиент <-> Сервер ===
add_executable(join_client 
    app/client/main.cpp
    app/client/network/client.cpp
    app/client/commands/command.cpp
    app/client/commands/command_executor.cpp
)

target_link_libraries(join_server PRIVATE sqlite3)
target_link_libraries(join_client PRIVATE sqlite3)


target_include_directories(join_server PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# линкуем Boost
target_link_libraries(join_server PUBLIC Boost::boost)  # Заголовочные зависимости
target_link_libraries(join_server PRIVATE Boost::system) # Библиотечные зависимости

# Настройки для Windows
if(WIN32)
    # Отключаем предупреждения для Boost и настраиваем Windows target
    target_compile_definitions(join_server PRIVATE _WIN32_WINNT=0x0601)
    target_compile_definitions(join_server PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Настройки именования библиотеки
set_target_properties(join_server PROPERTIES
    OUTPUT_NAME "join_server"
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
)

if(NOT WIN32)
    set_target_properties(join_server PROPERTIES PREFIX "lib")
endif()


target_include_directories(join_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Линкуем библиотеку Boost
target_link_libraries(join_client PRIVATE Boost::system)

if(WIN32)
    target_compile_definitions(join_server PRIVATE _WIN32_WINNT=0x0601)
endif()

target_include_directories(join_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(join_client PRIVATE Boost::system)

if(WIN32)
    target_compile_definitions(join_client PRIVATE _WIN32_WINNT=0x0601)
endif()

# === Настройки компилятора ===
# Универсальные настройки предупреждений
if(MSVC)
    target_compile_options(join_server PRIVATE /W4 /permissive-)
    target_compile_options(join_client PRIVATE /W4 /permissive-)
else()
    target_compile_options(join_server PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(join_client PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# === Настройки для Visual Studio ===
if(MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    
    # Группировка проектов в Solution Explorer
    set_target_properties(join_server PROPERTIES FOLDER "Applications")
    set_target_properties(join_client PROPERTIES FOLDER "Applications")
    
    # Установка стартового проекта
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                 PROPERTY VS_STARTUP_PROJECT join_server)
endif()

# === Установка ===
install(TARGETS join_server join_client
    EXPORT join_server-targets
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

# === CPack настройки ===
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "join_server")
set(CPACK_PACKAGE_VENDOR "Otus")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "join commands processing server")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "10")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_CONTACT "test@gmail.com")
set(CPACK_DEBIAN_PACKAGE_NAME "join_server")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Test User <test@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system1.74 (>= 1.74.0)")
set(CPACK_DEBIAN_FILE_NAME "join_server-${PROJECT_VERSION}-Linux.deb")

include(CPack)
