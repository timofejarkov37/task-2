cmake_minimum_required(VERSION 3.15)
project(hexdump C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/wd5045)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    link_libraries(msvcrt.lib)
    link_libraries(legacy_stdio_definitions.lib)
endif()

option(BUILD_SHARED_LIBS "Build libraries as shared (dynamic)" OFF)

add_library(hexdump_lib lib/hexdump.c)
target_include_directories(hexdump_lib PUBLIC include)
set_target_properties(hexdump_lib PROPERTIES OUTPUT_NAME hexdump)

add_executable(hexdump src/main.c src/getopt.c)
target_include_directories(hexdump PRIVATE include)
target_link_libraries(hexdump PRIVATE hexdump_lib)