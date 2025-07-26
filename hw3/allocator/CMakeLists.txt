cmake_minimum_required(VERSION 3.12)
set(PATCH_VERSION "3" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

project(allocator VERSION ${PROJECT_VESRION} LANGUAGES CXX)

cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

configure_file(version.h.in version.h)

add_library(allocator_ver lib.cpp)
add_executable(allocator_cli main.cpp )

set_target_properties(allocator_cli allocator_ver PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(allocator_ver
    PRIVATE "${CMAKE_BINARY_DIR}"
)

target_link_libraries(allocator_cli PRIVATE
    allocator_ver
)

if (MSVC)
    target_compile_options(allocator_cli PRIVATE /W4)
    target_compile_options(allocator_ver PRIVATE /W4)
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE /W4)
    endif()
else()
    target_compile_options(allocator_cli PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(allocator_ver PRIVATE -Wall -Wextra -pedantic -Werror)
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE -Wall -Wextra -pedantic -Werror)
    endif()
endif()

install(TARGETS allocator_cli RUNTIME DESTINATION bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_CONTACT "test@gmail.com")
include(CPack)
