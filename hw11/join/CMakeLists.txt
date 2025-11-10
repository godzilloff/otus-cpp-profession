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
)

if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
else()
    message(FATAL_ERROR "Boost not found! Please set BOOST_ROOT environment variable.")
endif()

# === SQLite3 ===
# Простой и надежный способ поиска SQLite3
find_path(SQLITE3_INCLUDE_DIR sqlite3.h)
find_library(SQLITE3_LIBRARY NAMES sqlite3)

if(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARY)
    message(STATUS "Found SQLite3: ${SQLITE3_LIBRARY}")
    message(STATUS "SQLite3 include dir: ${SQLITE3_INCLUDE_DIR}")
    
    add_library(sqlite3 INTERFACE IMPORTED)
    set_target_properties(sqlite3 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SQLITE3_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${SQLITE3_LIBRARY}"
    )
else()
    #message(FATAL_ERROR "SQLite3 not found! Please install libsqlite3-dev")
        
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
    
endif()

# === Google Test ===
if(BUILD_TESTING)
    find_package(GTest)
    if(GTest_FOUND)
        message(STATUS "GTest found: ${GTest_VERSION}")
        include(GoogleTest)
    else()
        message(WARNING "GTest not found - tests will be disabled")
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

# === Клиент join_client ===
add_executable(join_client 
    app/client/main.cpp
    app/client/network/client.cpp
    app/client/commands/command.cpp
    app/client/commands/command_executor.cpp
)

# Подключаем зависимости для сервера
target_link_libraries(join_server PRIVATE 
    Boost::system
    sqlite3
)

# Подключаем зависимости для клиента
target_link_libraries(join_client PRIVATE 
    Boost::system
    # Клиенту не нужен sqlite3, только серверу
)

# Настройки include директорий для сервера
target_include_directories(join_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app/server
)

# Настройки include директорий для клиента
target_include_directories(join_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app/client
)

# Настройки для Windows
if(WIN32)
    target_compile_definitions(join_server PRIVATE 
        _WIN32_WINNT=0x0601
        _CRT_SECURE_NO_WARNINGS
    )
    target_compile_definitions(join_client PRIVATE 
        _WIN32_WINNT=0x0601
        _CRT_SECURE_NO_WARNINGS
    )
endif()

# Настройки именования
set_target_properties(join_server PROPERTIES
    OUTPUT_NAME "join_server"
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
)

if(NOT WIN32)
    set_target_properties(join_server PROPERTIES PREFIX "lib")
endif()

# === Настройки компилятора ===
if(MSVC)
    target_compile_options(join_server PRIVATE /W4 /permissive-)
    target_compile_options(join_client PRIVATE /W4 /permissive-)
else()
    target_compile_options(join_server PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(join_client PRIVATE -Wall -Wextra -pedantic)
endif()

# === Настройки для Visual Studio ===
if(MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set_target_properties(join_server PROPERTIES FOLDER "Applications")
    set_target_properties(join_client PROPERTIES FOLDER "Applications")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                 PROPERTY VS_STARTUP_PROJECT join_server)
endif()

# === Установка ===
install(TARGETS join_server join_client
    RUNTIME DESTINATION bin
)

# === CPack настройки ===
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "join_server")
set(CPACK_PACKAGE_VENDOR "Otus")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "join commands processing server")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "11")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_CONTACT "test@gmail.com")
set(CPACK_DEBIAN_PACKAGE_NAME "join_server")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Test User <test@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system1.74 (>= 1.74.0), libsqlite3-0 (>= 3.31.0)")
set(CPACK_DEBIAN_FILE_NAME "join_server-${PROJECT_VERSION}-Linux.deb")

include(CPack)
