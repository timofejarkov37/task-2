/*
 * Реализация библиотеки вывода файлов в шестнадцатеричном виде.
 * Содержит реализацию всех вспомогательных функций: преобразование байт
 * в шестнадцатеричные символы, вывод строк дампа, обход директорий.
 *
 * Ярков Тимофей Сергеевич
 * МК-101
 */
#include "hexdump.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

static const char HEX_CHARS[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

long parse_nonneg_long(const char *str, const char *opt_name)
{
    char *end;
    long val = strtol(str, &end, 10);
    if (end == str || *end != '\0' || val < 0) {
        fprintf(stderr, "Error: invalid value for option -%s: '%s'\n",
                opt_name, str);
        exit(EXIT_FAILURE);
    }
    return val;
}

void options_init(Options *opts)
{
    opts->input_file = NULL;
    opts->offset     = 0;
    opts->length     = -1;
    opts->group_size = 1;
    opts->cols       = 16;
    opts->directory  = NULL;
}

void byte_to_hex(unsigned char byte, char out[2])
{
    out[0] = HEX_CHARS[(byte >> 4) & 0x0F];
    out[1] = HEX_CHARS[byte        & 0x0F];
}

int is_printable(unsigned char c)
{
    return (c >= 0x20 && c <= 0x7E);
}

