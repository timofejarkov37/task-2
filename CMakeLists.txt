# Минимальная версия CMake и название проекта
cmake_minimum_required(VERSION 3.15)
project(hexdump C)

# Стандарт C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Складываем все .exe и .dll в одну общую папку bin (чтобы Windows видел dll при запуске)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Настройки для MSVC (Windows)
if(MSVC)
    add_compile_options(/wd5045)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    link_libraries(msvcrt.lib)
    link_libraries(legacy_stdio_definitions.lib)
endif()

# Опция: статическая или динамическая библиотека
option(BUILD_SHARED_LIBS "Build libraries as shared (dynamic)" OFF)

# Подключаем подпроекты
add_subdirectory(lib)
add_subdirectory(app)