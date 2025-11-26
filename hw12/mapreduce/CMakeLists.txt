cmake_minimum_required(VERSION 3.15)
set(PATCH_VERSION "12" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.12.${PATCH_VERSION})
project(MapReduceNYC VERSION ${PROJECT_VERSION} LANGUAGES CXX)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

# Настройки компилятора
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive-")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -g")
endif()

# Опции проекта
option(BUILD_TESTS "Build tests" ON)
option(BUILD_PACKAGES "Build distribution packages" ON)

# Поиск зависимостей
find_package(PkgConfig)

# Включение тестов
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Создание структуры папок
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/input)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_output)

# Копирование входных данных
configure_file(${CMAKE_SOURCE_DIR}/input/AB_NYC_2019.csv ${CMAKE_BINARY_DIR}/input/AB_NYC_2019.csv COPYONLY)
#configure_file(${CMAKE_SOURCE_DIR}/input/test_data.csv ${CMAKE_BINARY_DIR}/input/test_data.csv COPYONLY)

# Добавление исполняемых файлов
add_executable(mapper_mean src/mapper_mean.cpp)
add_executable(reducer_mean src/reducer_mean.cpp)
add_executable(mapper_variance src/mapper_variance.cpp)
add_executable(reducer_variance src/reducer_variance.cpp)

# Установка выходной директории
set_target_properties(
    mapper_mean reducer_mean 
    mapper_variance reducer_variance
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Основная цель сборки
add_custom_target(build_all DEPENDS 
    mapper_mean reducer_mean 
    mapper_variance reducer_variance
)

# Интеграционные тесты
if(BUILD_TESTS)
    # Тест средней цены на реальных данных
    add_test(NAME integration_test_mean
        COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --build-and-test 
        ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} 
        --build-generator ${CMAKE_GENERATOR} --build-target run_mean_integration
    )
    
    # Тест дисперсии на реальных данных
    add_test(NAME integration_test_variance
        COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --build-and-test 
        ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} 
        --build-generator ${CMAKE_GENERATOR} --build-target run_variance_integration
    )


endif()
# === CPack настройки для создания пакетов ===
if(BUILD_PACKAGES)
    include(InstallRequiredSystemLibraries)
    
    # Общие настройки пакета
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "mapreduce-nyc")
    set(CPACK_PACKAGE_VENDOR "Otus C++ Professional")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MapReduce implementation for NYC real estate data analysis")
    set(CPACK_PACKAGE_DESCRIPTION "A C++ MapReduce implementation that calculates mean price and variance for NYC real estate data using the MapReduce paradigm.")
    set(CPACK_PACKAGE_VERSION_MAJOR "0")
    set(CPACK_PACKAGE_VERSION_MINOR "12")
    set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
    set(CPACK_PACKAGE_CONTACT "test@mail.ru")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "MapReduceNYC")
    
    # Настройки для DEB пакетов (Ubuntu/Debian)
    set(CPACK_DEBIAN_PACKAGE_NAME "mapreduce-nyc")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kosilov github.com/godzilloff")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6 (>= 9.0), libgcc-s1 (>= 9.0)")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/godzilloff/otus-cpp-profession/tree/hw12")
    set(CPACK_DEBIAN_FILE_NAME "mapreduce-nyc-${PROJECT_VERSION}-Linux.deb")
    
    # Установка файлов в пакет
    install(TARGETS 
        mapper_mean reducer_mean 
        mapper_variance reducer_variance
        RUNTIME DESTINATION bin
    )
    
    # Установка данных и скриптов
    install(
        FILES 
            input/AB_NYC_2019.csv
            scripts/run_locally.sh
            scripts/test_calculation.py
        DESTINATION share/mapreduce-nyc
    )
    
    # Включение CPack
    include(CPack)
    
    # Цель для создания пакетов
    add_custom_target(packages
        COMMAND ${CMAKE_CPACK_COMMAND} --config ${CMAKE_BINARY_DIR}/CPackConfig.cmake
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS build_all
        COMMENT "Building distribution packages"
    )
    
    message(STATUS "Package building enabled: DEB, RPM, TGZ")
endif()

# Информация о проекте
message(STATUS "==========================================")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Build packages: ${BUILD_PACKAGES}")
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/bin")
message(STATUS "==========================================")
