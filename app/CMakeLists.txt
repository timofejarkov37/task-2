# Исполняемый файл приложения
add_executable(hexdump
    src/main.c
    src/getopt.c
)

# Приватные заголовки (только для этого исполняемого файла, здесь лежит getopt.h)
target_include_directories(hexdump PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Линкуем с нашей библиотекой
target_link_libraries(hexdump PRIVATE hexdump_lib)