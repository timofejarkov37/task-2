/*
 * Заголовочный файл библиотеки вывода файлов в шестнадцатеричном виде.
 *
 * Ярков Тимофей Сергеевич
 * МК-101
 */

#ifndef HEXDUMP_H
#define HEXDUMP_H

#include <stdio.h>

typedef struct {
    const char *input_file; /* -i: имя входного файла                        */
    long        offset;     /* -o: смещение от начала файла (байт)           */
    long        length;     /* -l: количество выводимых байт (-1 = до конца) */
    int         group_size; /* -g: размер кусочка (байт)                     */
    int         cols;       /* -n: количество кусочков в строке              */
    const char *directory;  /* -d: директория для вывода всех файлов         */
} Options;

void options_init(Options *opts);
void byte_to_hex(unsigned char byte, char out[2]);
void print_hex(FILE *fp, const Options *opts);
int  print_file(const char *path, const Options *opts);
int  print_dir(const char *dir_path, const Options *opts);
int  is_printable(unsigned char c);
long parse_nonneg_long(const char *str, const char *opt_name);

#endif
