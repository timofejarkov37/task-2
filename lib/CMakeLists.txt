# Библиотека с основными функциями hexdump
add_library(hexdump_lib src/hexdump.c)

# Публичные заголовки (доступны тем, кто линкуется с библиотекой)
target_include_directories(hexdump_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Имя выходного файла 
set_target_properties(hexdump_lib PROPERTIES OUTPUT_NAME hexdump)

# Убираем приставку "lib" для Windows (будет hexdump.dll, а не libhexdump.dll)
set_target_properties(hexdump_lib PROPERTIES PREFIX "")

# При сборке DLL на Windows добавляем lib.def для экспорта функций
if(WIN32 AND BUILD_SHARED_LIBS)
    target_sources(hexdump_lib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib.def")
endif()