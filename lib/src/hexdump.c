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

static void print_group(const unsigned char *buf, int start,
                       int valid, int group_size)
{
    int i;
    for (i = group_size - 1; i >= 0; i--) {
        int idx = start + i;
        if (idx < valid) {
            char h[2];
            byte_to_hex(buf[idx], h);
            putchar(h[0]);
            putchar(h[1]);
        } else {
            putchar('0');
            putchar('0');
        }
    }
}

static void print_ascii(const unsigned char *buf, int valid)
{
    int i;
    printf(" | ");
    for (i = 0; i < valid; i++) {
        if (is_printable(buf[i]))
            putchar((char)buf[i]);
        else
            putchar('.');
    }
}

static void print_offset(long pos)
{
    char digits[8];
    int i;
    unsigned long val = (unsigned long)pos;
    for (i = 7; i >= 0; i--) {
        digits[i] = HEX_CHARS[val & 0x0F];
        val >>= 4;
    }
    for (i = 0; i < 8; i++)
        putchar(digits[i]);
}

void print_hex(FILE *fp, const Options *opts)
{
    int  group_size = opts->group_size;
    int  cols       = opts->cols;
    long pos        = opts->offset;
    long length     = opts->length;

    int row_bytes = group_size * cols;

    unsigned char *buf = (unsigned char *)malloc((size_t)row_bytes);
    if (!buf) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return;
    }

    if (fseek(fp, pos, SEEK_SET) != 0) {
        fprintf(stderr, "Error: cannot seek to offset %ld\n", pos);
        free(buf);
        return;
    }

    long written = 0;

    while (1) {
        int to_read = row_bytes;
        if (length >= 0) {
            long remaining = length - written;
            if (remaining <= 0) break;
            if (remaining < to_read) to_read = (int)remaining;
        }

        int valid = (int)fread(buf, 1, (size_t)to_read, fp);
        if (valid == 0) break;

        print_offset(pos);
        printf("  ");

        int ngroups = valid / group_size + (valid % group_size != 0 ? 1 : 0);

        int g;
        for (g = 0; g < cols; g++) {
            int byte_pos = g * group_size;
            if (byte_pos < valid) {
                print_group(buf, byte_pos, valid, group_size);
                if (g < cols - 1) {
                    if (group_size == 1 || g < ngroups - 1)
                        putchar(' ');
                }
            } else if (group_size == 1) {
                putchar(' ');
                putchar(' ');
                if (g < cols - 1) putchar(' ');
            }
        }

        if (group_size == 1)
            print_ascii(buf, valid);

        putchar('\n');

        written += valid;
        pos     += valid;

        if (valid < to_read) break;
    }

    free(buf);
}

int print_file(const char *path, const Options *opts)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file '%s'\n", path);
        return -1;
    }
    print_hex(fp, opts);
    fclose(fp);
    return 0;
}

int print_dir(const char *dir_path, const Options *opts)
{
    DIR *dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "Error: cannot open directory '%s'\n", dir_path);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        size_t len = strlen(dir_path) + 1 + strlen(entry->d_name) + 1;
        char *fpath = (char *)malloc(len);
        if (!fpath) {
            fprintf(stderr, "Error: memory allocation failed\n");
            continue;
        }
        snprintf(fpath, len, "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(fpath, &st) == 0 && S_ISREG(st.st_mode)) {
            printf("=== %s ===\n", fpath);
            print_file(fpath, opts);
        }

        free(fpath);
    }

    closedir(dir);
    return 0;
}