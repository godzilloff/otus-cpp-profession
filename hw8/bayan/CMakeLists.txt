cmake_minimum_required(VERSION 3.12)
set(PATCH_VERSION "8" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.8.${PATCH_VERSION})

project(bayan VERSION ${PROJECT_VERSION} LANGUAGES CXX)
enable_testing()

cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

if (MSVC AND NOT GTEST_FOUND)
    set(GTEST_ROOT "c:/lib/googletest/" CACHE PATH "Path to Google Test")
endif()

set(BOOST_ROOT $ENV{BOOST_ROOT})
set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR})
set(CMAKE_PREFIX_PATH "${BOOST_ROOT};${CMAKE_PREFIX_PATH}")

# === BOOST ===
# Добавляем все необходимые компоненты Boost
find_package(Boost REQUIRED COMPONENTS 
    filesystem 
    program_options 
    iostreams 
    system
)

message(STATUS "Boost found: ${Boost_FOUND}")
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "Boost not found! Please set BOOST_ROOT environment variable.")
endif()

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(bayan 
    src/main.cpp
    src/duplicate_finder.cpp 
    src/duplicate_finder.h)

add_executable(bayan_test 
    test/main.cpp
    src/duplicate_finder.cpp
    src/duplicate_finder.h
    test/test_file_hasher.cpp
    test/test_duplicate_finder.cpp)

set_target_properties(bayan bayan_test 
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Линковка для основного исполняемого файла bayan
target_link_libraries(bayan PRIVATE
    Boost::filesystem
    Boost::program_options
    Boost::iostreams
    Boost::system
)

# Линковка для тестового исполняемого файла
target_include_directories(
    bayan_test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(
    bayan_test PRIVATE
    GTest::gtest_main
    Boost::filesystem
    Boost::program_options
    Boost::iostreams
    Boost::system
)

message(STATUS "GTest include dirs: ${GTEST_INCLUDE_DIRS}")
message(STATUS "GTest libraries: ${GTEST_BOTH_LIBRARIES}")

if (MSVC)
    target_compile_options(bayan PRIVATE /W4)
    target_compile_options(bayan_test PRIVATE /W4)

    target_link_libraries(
        bayan_test PRIVATE
        "c:/lib/googletest/lib/gtest.lib"
    )

else()
    target_compile_options(bayan PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(bayan_test PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

if (MSVC)
# === hide VS projects: ALL_BUILD and ZERO_CHECK ===
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# === assign it as a startup project VS ===
set_property(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
    PROPERTY VS_STARTUP_PROJECT bayan
)
endif ()

# Автоматическое обнаружение тестов
gtest_discover_tests(bayan_test)

# Для генерации XML-отчётов
set(CTEST_OUTPUT_ON_FAILURE ON)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/test_results")
set(GTEST_TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test_results")

install(TARGETS bayan RUNTIME DESTINATION bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "bayan")
set(CPACK_DEBIAN_PACKAGE_NAME "bayan")
set(CPACK_DEBIAN_FILE_NAME "bayan-${PROJECT_VERSION}-Linux.deb")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "8")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_CONTACT "test@gmail.com")
include(CPack)
