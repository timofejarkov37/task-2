/*
 * Главный файл программы вывода содержимого файлов в шестнадцатеричном виде.
 *
 * Ярков Тимофей Сергеевич
 * МК-101
 */

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"   /* для разбора аргументов командной строки */
#include "hexdump.h"  /* наша библиотека с функциями печати */

/* argc — количество аргументов командной строки (включая имя программы)
 * argv — массив строк с самими аргументами */
int main(int argc, char *argv[])
{
    Options opts;             /* структура с настройками программы */
    options_init(&opts);      /* заполняем её значениями по умолчанию */

    int c;  /* текущий разобранный символ опции */
    
    /* getopt автоматически разбирает аргументы вида -o 5 -i file
     * Строка "i:o:l:g:n:d:" описывает, какие опции мы поддерживаем.
     * Двоеточие после буквы означает, что опция требует аргумент (например, -i file). */
    while ((c = getopt(argc, argv, "i:o:l:g:n:d:")) != -1) {
        switch (c) {
            case 'i':
                /* optarg — это глобальная переменная getopt,
                 * в неё попадает строка-аргумент текущей опции */
                opts.input_file = optarg;
                break;

            case 'o':
                opts.offset = parse_nonneg_long(optarg, "o");
                break;

            case 'l':
                opts.length = parse_nonneg_long(optarg, "l");
                break;

            case 'g':
                opts.group_size = (int)parse_nonneg_long(optarg, "g");
                if (opts.group_size == 0) {
                    fprintf(stderr, "Error: chunk size (-g) must be > 0\n");
                    return EXIT_FAILURE;
                }
                break;

            case 'n':
                opts.cols = (int)parse_nonneg_long(optarg, "n");
                if (opts.cols == 0) {
                    fprintf(stderr, "Error: chunk count (-n) must be > 0\n");
                    return EXIT_FAILURE;
                }
                break;

            case 'd':
                opts.directory = optarg;
                break;

            case '?':
                /* getopt вернул '?', если встретил неизвестную опцию
                 * или опцию без обязательного аргумента.
                 * Сообщение об ошибке getopt печатает сам. */
                return EXIT_FAILURE;

            default:
                return EXIT_FAILURE;
        }
    }

    /* Проверка: нельзя одновременно указать и файл, и директорию */
    if (opts.directory != NULL && opts.input_file != NULL) {
        fprintf(stderr, "Error: -i and -d cannot be used together\n");
        return EXIT_FAILURE;
    }

    /* Если указана директория — выводим все файлы из неё */
    if (opts.directory != NULL)
        return print_dir(opts.directory, &opts) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;

    /* Если указан конкретный файл — выводим только его */
    if (opts.input_file != NULL)
        return print_file(opts.input_file, &opts) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;

    /* Если пользователь не указал ни -i, ни -d — сообщаем об ошибке */
    fprintf(stderr, "Error: specify -i <file> or -d <directory>\n");
    return EXIT_FAILURE;
}