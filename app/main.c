/*
 * Главный файл программы вывода содержимого файлов в шестнадцатеричном виде.
 *
 * Ярков Тимофей Сергеевич
 * МК-101
 */

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"
#include "hexdump.h"

int main(int argc, char *argv[])
{
    Options opts;
    options_init(&opts);

    int c;
    while ((c = getopt(argc, argv, "i:o:l:g:n:d:")) != -1) {
        switch (c) {
            case 'i':
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
                return EXIT_FAILURE;

            default:
                return EXIT_FAILURE;
        }
    }

    if (opts.directory != NULL && opts.input_file != NULL) {
        fprintf(stderr, "Error: -i and -d cannot be used together\n");
        return EXIT_FAILURE;
    }

    if (opts.directory != NULL)
        return print_dir(opts.directory, &opts) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;

    if (opts.input_file != NULL)
        return print_file(opts.input_file, &opts) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;

    fprintf(stderr, "Error: specify -i <file> or -d <directory>\n");
    return EXIT_FAILURE;
}