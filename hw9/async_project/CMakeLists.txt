cmake_minimum_required(VERSION 3.12)
set(PATCH_VERSION "9" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.9.${PATCH_VERSION})
project(async VERSION ${PROJECT_VERSION} LANGUAGES CXX)

cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Для Windows: определяем ASYNC_EXPORTS при сборке библиотеки
add_library(async SHARED 
    src/async.cpp 
    src/command_processor.cpp
)

target_include_directories(async PUBLIC include)

if(WIN32)
    target_compile_definitions(async PRIVATE ASYNC_EXPORTS)
    set_target_properties(async PROPERTIES
        OUTPUT_NAME "async"
        PREFIX ""
    )
else()
    set_target_properties(async PROPERTIES
        OUTPUT_NAME "async"
        PREFIX "lib"
    )
endif()

# Исполняемый файл
add_executable(async_demo 
    app/main.cpp
)

target_link_libraries(async_demo async)

if (MSVC)
    target_compile_options(async PRIVATE /W4)
    target_compile_options(async_demo PRIVATE /W4)
else()
    target_compile_options(async PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(async_demo PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

if (MSVC)
# === hide VS projects: ALL_BUILD and ZERO_CHECK ===
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# === assign it as a startup project VS ===
set_property(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
    PROPERTY VS_STARTUP_PROJECT async_demo
)
endif ()

# Установка
install(TARGETS async async_demo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)


set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "async_demo")
set(CPACK_DEBIAN_PACKAGE_NAME "async_demo")
set(CPACK_DEBIAN_FILE_NAME "async_demo-${PROJECT_VERSION}-Linux.deb")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "9")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_CONTACT "test@gmail.com")
include(CPack)
