cmake_minimum_required(VERSION 3.12)
set(PATCH_VERSION "6" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.6.${PATCH_VERSION})

project(matrix VERSION ${PROJECT_VERSION} LANGUAGES CXX)
enable_testing()

cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

if (MSVC AND NOT GTEST_FOUND)
    set(GTEST_ROOT "c:/lib/googletest/" CACHE PATH "Path to Google Test")
endif()

# Проверка что мы в GitHub Actions
if(DEFINED ENV{GITHUB_ACTIONS})
    message(STATUS "Running in GitHub Actions")
    find_package(GTest REQUIRED PATHS $ENV{HOME}/.local/gtest NO_DEFAULT_PATH)
    set(IN_GITHUB_ACTIONS TRUE)
else()
    find_package(GTest REQUIRED)
    set(IN_GITHUB_ACTIONS FALSE)
endif()

include(GoogleTest)

add_executable(matrix_cli 
    src/main.cpp )

add_executable(matrix_test 
    test/main.cpp
    test/matrix_tests.cpp )

set_target_properties(matrix_cli matrix_test 
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add src to the include directories for the test target
target_include_directories(
    matrix_test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(
    matrix_test PRIVATE
    GTest::gtest_main
)

message(STATUS ${GTEST_INCLUDE_DIRS})
message(STATUS ${GTEST_BOTH_LIBRARIES})

if (MSVC)
    target_compile_options(matrix_cli PRIVATE /W4)
    target_compile_options(matrix_test PRIVATE /W4)

    target_link_libraries(
        matrix_test PRIVATE
        "c:/lib/googletest/lib/gtest.lib"
    )

else()
    target_compile_options(matrix_cli PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(matrix_test PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

if (MSVC)
# === hide VS projects: ALL_BUILD and ZERO_CHECK ===
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# === assign it as a startup project VS ===
set_property(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
    PROPERTY VS_STARTUP_PROJECT matrix_cli
)
endif ()

# Автоматическое обнаружение тестов
gtest_discover_tests(matrix_test)

# Для генерации XML-отчётов
set(CTEST_OUTPUT_ON_FAILURE ON)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/test_results")
set(GTEST_TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test_results")

install(TARGETS matrix_cli RUNTIME DESTINATION bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "6")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_CONTACT "test@gmail.com")
include(CPack)
