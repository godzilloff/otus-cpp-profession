cmake_minimum_required(VERSION 3.12)
set(PATCH_VERSION "4" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.4.${PATCH_VERSION})

project(print_ip VERSION ${PROJECT_VESRION} LANGUAGES CXX)

cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

configure_file(version.h.in version.h)

add_library(print_ip_ver lib.cpp)
add_executable(print_ip_cli main.cpp )

set_target_properties(print_ip_cli print_ip_ver PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(print_ip_ver
    PRIVATE "${CMAKE_BINARY_DIR}"
)

target_link_libraries(print_ip_cli PRIVATE
    print_ip_ver
)

if (MSVC)
    target_compile_options(print_ip_cli PRIVATE /W4)
    target_compile_options(print_ip_ver PRIVATE /W4)
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE /W4)
    endif()
else()
    target_compile_options(print_ip_cli PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(print_ip_ver PRIVATE -Wall -Wextra -pedantic -Werror)
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE -Wall -Wextra -pedantic -Werror)
    endif()
endif()

install(TARGETS print_ip_cli RUNTIME DESTINATION bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_CONTACT "test@gmail.com")
include(CPack)
