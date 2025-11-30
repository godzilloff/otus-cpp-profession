cmake_minimum_required(VERSION 3.15)
set(PATCH_VERSION "13" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.13.${PATCH_VERSION})
project(FashionMNIST VERSION ${PROJECT_VERSION} 
	DESCRIPTION "Fashion MNIST Classifier using MLP"
	LANGUAGES CXX)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Политики CMake
cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

# ========== Eigen Configuration ==========
set(EIGEN3_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/eigen)

if(NOT EXISTS ${EIGEN3_INCLUDE_DIR})
    message(FATAL_ERROR "Eigen not found in ${EIGEN3_INCLUDE_DIR}")
endif()

add_library(eigen INTERFACE)
target_include_directories(eigen INTERFACE ${EIGEN3_INCLUDE_DIR})

# ========== Compiler Settings ==========
if(MSVC)
    target_compile_options(eigen INTERFACE /W4 /permissive-)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
else()
    target_compile_options(eigen INTERFACE -Wall -Wextra -pedantic -g)
endif()

# Опции проекта
option(BUILD_TESTS "Build tests" ON)
option(BUILD_PACKAGES "Build distribution packages" ON)

# Поиск зависимостей
#find_package(PkgConfig)

# Включение тестов
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ========== Include Directories ==========
include_directories(${CMAKE_SOURCE_DIR}/include)

# ========== Main Executable ==========
add_executable(fashion_mnist 
    src/main.cpp
    src/MLPModel.cpp
    src/CSVReader.cpp
)

target_link_libraries(fashion_mnist eigen)

# ========== Data Files Handling ==========
# Копируем тестовые данные в выходную директорию (опционально)
#file(COPY input/ DESTINATION ${CMAKE_SOURCE_DIR}/input)
file(COPY input/ DESTINATION ${CMAKE_BINARY_DIR}/Debug/input
)

# === CPack настройки для создания пакетов ===
if(BUILD_PACKAGES)
    include(InstallRequiredSystemLibraries)
    
    # Общие настройки пакета
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "fashion_mnist")
    set(CPACK_PACKAGE_VENDOR "Otus C++ Professional")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fashion MNIST Classifier")
    set(CPACK_PACKAGE_DESCRIPTION "C++ implementation of MLP model for Fashion MNIST classification")
    set(CPACK_PACKAGE_VERSION_MAJOR "0")
    set(CPACK_PACKAGE_VERSION_MINOR "13")
    set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
    set(CPACK_PACKAGE_CONTACT "test@mail.ru")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "FashionMNIST")
    
    # Настройки для DEB пакетов (Ubuntu/Debian)
    set(CPACK_DEBIAN_PACKAGE_NAME "fashion-mnist")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kosilov github.com/godzilloff")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6 (>= 9.0), libgcc-s1 (>= 9.0)")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/godzilloff/otus-cpp-profession/tree/hw13")
    set(CPACK_DEBIAN_FILE_NAME "fashion-mnist-${PROJECT_VERSION}-Linux.deb")
    
    # Установка файлов в пакет
    install(TARGETS 
        fashion_mnist
        RUNTIME DESTINATION bin
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
    
    message(STATUS "Package building enabled")
endif()

# ========== Project Information ==========
message(STATUS "==========================================")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Description: ${PROJECT_DESCRIPTION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Build packages: ${BUILD_PACKAGES}")
message(STATUS "Source dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Build dir: ${CMAKE_BINARY_DIR}")
message(STATUS "==========================================")
