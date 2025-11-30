# Настройка тестов для Fashion MNIST
set(TEST_SOURCES
    test_main.cpp
    test_csv_reader.cpp
    test_matrix_utils.cpp
    test_mlp_model.cpp
    test_integration.cpp
)

# Добавляем исходные файлы проекта, которые тестируем
set(PROJECT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/MLPModel.cpp
    ${CMAKE_SOURCE_DIR}/src/CSVReader.cpp
)

# Создание исполняемого файла тестов
add_executable(fashion_mnist_tests
    ${TEST_SOURCES}
    ${PROJECT_SOURCES}
)

# Подключение Google Test с поддержкой локальной установки и GitHub Actions
if(WIN32)
    # Для Windows - локальная установка
    set(GTEST_ROOT "C:/lib/googletest" CACHE PATH "Path to Google Test")
    
    # Поиск в стандартных путях Windows
    find_path(GTEST_INCLUDE_DIR gtest/gtest.h
        PATHS
        ${GTEST_ROOT}/include
        "C:/lib/googletest/include"
        "C:/Program Files/googletest/include"
        DOC "Google Test include directory"
    )
    
    find_library(GTEST_LIBRARY
        NAMES gtest gtestd
        PATHS
        ${GTEST_ROOT}/lib
        ${GTEST_ROOT}/lib/x64
        "C:/lib/googletest/lib"
        "C:/lib/googletest/lib/x64"
        DOC "Google Test library"
    )
    
    find_library(GTEST_MAIN_LIBRARY
        NAMES gtest_main gtest_maind
        PATHS
        ${GTEST_ROOT}/lib
        ${GTEST_ROOT}/lib/x64
        "C:/lib/googletest/lib"
        "C:/lib/googletest/lib/x64"
        DOC "Google Test main library"
    )
    
    find_library(GMOCK_LIBRARY
        NAMES gmock gmockd
        PATHS
        ${GTEST_ROOT}/lib
        ${GTEST_ROOT}/lib/x64
        "C:/lib/googletest/lib"
        "C:/lib/googletest/lib/x64"
        DOC "Google Mock library"
    )
    
    if(GTEST_INCLUDE_DIR AND GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
        message(STATUS "Found local Google Test: ${GTEST_ROOT}")
        set(GTEST_FOUND TRUE)
        
        # Создаем импортируемые цели
        add_library(GTest::gtest UNKNOWN IMPORTED)
        add_library(GTest::gtest_main UNKNOWN IMPORTED)
        add_library(GTest::gmock UNKNOWN IMPORTED)
        
        set_target_properties(GTest::gtest PROPERTIES
            IMPORTED_LOCATION "${GTEST_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIR}"
        )
        
        set_target_properties(GTest::gtest_main PROPERTIES
            IMPORTED_LOCATION "${GTEST_MAIN_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "GTest::gtest"
        )
        
        set_target_properties(GTest::gmock PROPERTIES
            IMPORTED_LOCATION "${GMOCK_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "GTest::gtest"
        )
        
    else()
        message(WARNING "Local Google Test not found at ${GTEST_ROOT}, trying system package...")
        find_package(GTest QUIET)
    endif()
    
else()
    # Для Linux (GitHub Actions) - используем системный пакет
    find_package(GTest REQUIRED)
endif()

# Если не нашли локальный GTest, используем стандартный поиск
if(NOT GTEST_FOUND)
    find_package(GTest REQUIRED)
endif()

if(WIN32 AND EXISTS "C:/lib/googletest")
    # Явно добавляем пути для IntelliSense
    target_include_directories(fashion_mnist_tests PRIVATE "C:/lib/googletest/include")
    message(STATUS "Added explicit include path for Google Test: C:/lib/googletest/include")
endif()

include(GoogleTest)

# Включение директорий
target_include_directories(fashion_mnist_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(GTEST_INCLUDE_DIR)
    target_include_directories(fashion_mnist_tests PRIVATE ${GTEST_INCLUDE_DIR})
endif()

# Линковка с Google Test и основными библиотеками проекта
if(WIN32 AND GTEST_FOUND)
    # Для локальной установки на Windows
    target_link_libraries(fashion_mnist_tests PRIVATE 
        GTest::gtest_main
        GTest::gtest
        GTest::gmock
        eigen
    )
else()
    # Для системной установки (Linux/Ubuntu)
    target_link_libraries(fashion_mnist_tests PRIVATE 
        GTest::gtest_main
        eigen
    )
endif()

# Настройки компилятора
if(MSVC)
    target_compile_options(fashion_mnist_tests PRIVATE 
        /W4
        /D_CRT_SECURE_NO_WARNINGS
        /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
    )
    target_compile_definitions(fashion_mnist_tests PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
    )
else()
    target_compile_options(fashion_mnist_tests PRIVATE 
        -Wall
        -Wextra
        -Wpedantic
        -g
    )
endif()

# Копирование тестовых данных
# file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/test_data)
# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_data/ 
#      DESTINATION ${CMAKE_BINARY_DIR}/tests/test_data
# )

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
file(COPY ${CMAKE_SOURCE_DIR}/input/
     DESTINATION ${CMAKE_BINARY_DIR}/tests/input
)


# Добавление тестов
gtest_discover_tests(fashion_mnist_tests)

# Также добавляем явные тесты для совместимости
add_test(NAME test_csv_reader COMMAND fashion_mnist_tests --gtest_filter="CSVReaderTest*")
add_test(NAME test_matrix_utils COMMAND fashion_mnist_tests --gtest_filter="MatrixUtilsTest*")
add_test(NAME test_mlp_model COMMAND fashion_mnist_tests --gtest_filter="MLPModelTest*")
add_test(NAME test_integration COMMAND fashion_mnist_tests --gtest_filter="IntegrationTest*")
add_test(NAME test_all COMMAND fashion_mnist_tests)

# Установка переменных окружения для тестов
set_tests_properties(
    test_csv_reader 
    test_matrix_utils 
    test_mlp_model 
    test_integration 
    test_all
    PROPERTIES 
    ENVIRONMENT "TEST_DATA_DIR=${CMAKE_BINARY_DIR}/tests/test_data"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Цель для запуска unit тестов
add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -V --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS fashion_mnist_tests
    COMMENT "Running unit tests with verbose output"
)

# Цель для запуска конкретных групп тестов
add_custom_target(run_csv_tests
    COMMAND ${CMAKE_BINARY_DIR}/fashion_mnist_tests --gtest_filter="CSVReaderTest*" --gtest_output=xml
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS fashion_mnist_tests
    COMMENT "Running CSV reader tests"
)

add_custom_target(run_mlp_tests
    COMMAND ${CMAKE_BINARY_DIR}/fashion_mnist_tests --gtest_filter="MLPModelTest*" --gtest_output=xml
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS fashion_mnist_tests
    COMMENT "Running MLP model tests"
)

add_custom_target(run_integration_tests
    COMMAND ${CMAKE_BINARY_DIR}/fashion_mnist_tests --gtest_filter="IntegrationTest*" --gtest_output=xml
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS fashion_mnist_tests
    COMMENT "Running integration tests"
)

# Цель для полной сборки проекта с тестами
add_custom_target(build_and_test
    COMMAND ${CMAKE_COMMAND} --build . --config $<CONFIG> --target fashion_mnist_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -V --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building project and running all tests"
)

# Цель для генерации отчетов тестов
add_custom_target(test_reports
    COMMAND ${CMAKE_BINARY_DIR}/fashion_mnist_tests --gtest_output=xml:test_report.xml
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS fashion_mnist_tests
    COMMENT "Generating test reports in XML format"
)

# Информация о конфигурации тестов
message(STATUS "Google Test configuration:")
message(STATUS "  GTEST_ROOT: ${GTEST_ROOT}")
message(STATUS "  GTEST_INCLUDE_DIR: ${GTEST_INCLUDE_DIR}")
message(STATUS "  GTEST_LIBRARY: ${GTEST_LIBRARY}")
message(STATUS "  GTEST_MAIN_LIBRARY: ${GTEST_MAIN_LIBRARY}")
message(STATUS "  Test binary: ${CMAKE_BINARY_DIR}/fashion_mnist_tests")
message(STATUS "  Test data directory: ${CMAKE_BINARY_DIR}/tests/test_data")
