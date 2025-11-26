# Настройка тестов
set(TEST_SOURCES
    test_utils.cpp  # Добавляем файл с реализациями
    test_mappers.cpp
    test_reducers.cpp
    test_integration.cpp
)

# Создание исполняемого файла тестов
add_executable(mapreduce_tests ${TEST_SOURCES})

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

include(GoogleTest)

# Включение директорий
target_include_directories(mapreduce_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}  # Для test_utils.h
)

if(GTEST_INCLUDE_DIR)
    target_include_directories(mapreduce_tests PRIVATE ${GTEST_INCLUDE_DIR})
endif()

# Линковка с Google Test
if(WIN32 AND GTEST_FOUND)
    # Для локальной установки на Windows
    target_link_libraries(mapreduce_tests PRIVATE 
        GTest::gtest_main
        GTest::gtest
        GTest::gmock
    )
else()
    # Для системной установки (Linux/Ubuntu)
    target_link_libraries(mapreduce_tests PRIVATE 
        GTest::gtest_main
    )
endif()

# Настройки компилятора
if(MSVC)
    target_compile_options(mapreduce_tests PRIVATE 
        /W4
        /D_CRT_SECURE_NO_WARNINGS
        /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
    )
    target_compile_definitions(mapreduce_tests PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
    )
else()
    target_compile_options(mapreduce_tests PRIVATE 
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Копирование тестовых данных
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_data)
configure_file(${CMAKE_SOURCE_DIR}/tests/test_data/sample_input.csv ${CMAKE_BINARY_DIR}/test_data/sample_input.csv COPYONLY)

# Добавление тестов
gtest_discover_tests(mapreduce_tests)

# Также добавляем явные тесты для совместимости
add_test(NAME unit_test_mappers COMMAND mapreduce_tests --gtest_filter="MapperTest*")
add_test(NAME unit_test_reducers COMMAND mapreduce_tests --gtest_filter="ReducerTest*")
add_test(NAME unit_test_integration COMMAND mapreduce_tests --gtest_filter="IntegrationTest*")
add_test(NAME unit_test_all COMMAND mapreduce_tests)

# Установка переменных окружения для тестов
set_tests_properties(unit_test_mappers unit_test_reducers unit_test_integration unit_test_all
    PROPERTIES ENVIRONMENT "TEST_DATA_DIR=${CMAKE_BINARY_DIR}/test_data"
)

# Цель для запуска unit тестов
add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -V
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS mapreduce_tests
    COMMENT "Running unit tests"
)

# Интеграционные тесты
add_custom_target(run_mean_integration
    COMMAND ${CMAKE_COMMAND} -E echo "Running mean integration test..."
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/run_mean.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS mapper_mean reducer_mean
)

add_custom_target(run_variance_integration
    COMMAND ${CMAKE_COMMAND} -E echo "Running variance integration test..."
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/run_variance.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS mapper_variance reducer_variance
)

add_custom_target(run_all_tests
    COMMAND ${CMAKE_COMMAND} -E echo "Running all tests..."
    COMMAND ${CMAKE_CTEST_COMMAND} -V
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS mapreduce_tests
    COMMENT "Running all tests"
)

# Цель для полной сборки проекта с тестами
add_custom_target(build_and_test
    COMMAND ${CMAKE_COMMAND} --build . --target build_all
    COMMAND ${CMAKE_COMMAND} --build . --target run_all_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building project and running all tests"
)

# Информация о конфигурации тестов
message(STATUS "Google Test configuration:")
message(STATUS "  GTEST_ROOT: ${GTEST_ROOT}")
message(STATUS "  GTEST_INCLUDE_DIR: ${GTEST_INCLUDE_DIR}")
message(STATUS "  GTEST_LIBRARY: ${GTEST_LIBRARY}")
message(STATUS "  GTEST_MAIN_LIBRARY: ${GTEST_MAIN_LIBRARY}")
