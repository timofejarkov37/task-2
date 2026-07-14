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

/* Массив для быстрого перевода числа от 0 до 15 в соответствующий HEX-символ */
static const char HEX_CHARS[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/* Преобразует строку в неотрицательное число.
 * Если строка не является числом или число отрицательное — завершает программу с ошибкой. */
long parse_nonneg_long(const char *str, const char *opt_name)
{
    char *end;       /* указатель на первый символ после числа (для проверки корректности) */
    long val = strtol(str, &end, 10);  /* само прочитанное число */
    if (end == str || *end != '\0' || val < 0) {
        fprintf(stderr, "Error: invalid value for option -%s: '%s'\n",
                opt_name, str);
        exit(EXIT_FAILURE);
    }
    return val;
}

/* Задает значения по умолчанию для всех настроек программы */
void options_init(Options *opts)
{
    opts->input_file = NULL;
    opts->offset     = 0;
    opts->length     = -1;
    opts->group_size = 1;
    opts->cols       = 16;
    opts->directory  = NULL;
}

/* Переводит один байт в два HEX-символа вручную (без использования sprintf) */
void byte_to_hex(unsigned char byte, char out[2])
{
    out[0] = HEX_CHARS[(byte >> 4) & 0x0F];
    out[1] = HEX_CHARS[byte        & 0x0F];
}

/* Проверяет, является ли символ печатным (английские буквы, цифры, знаки препинания) */
int is_printable(unsigned char c)
{
    return (c >= 0x20 && c <= 0x7E);
}

/* Выводит одну группу байт в HEX-виде.
 * Байты выводятся справа налево (как требуется в примерах из задания).
 * Если в конце файла байт не хватает для полной группы, они дополняются нулями. */
static void print_group(const unsigned char *buf, int start,
                       int valid, int group_size)
{
    int i;  /* номер байта внутри группы (итерация от последнего к первому) */
    for (i = group_size - 1; i >= 0; i--) {
        int idx = start + i;  /* реальный индекс байта в буфере */
        if (idx < valid) {
            char h[2];  /* два HEX-символа для одного байта */
            byte_to_hex(buf[idx], h);
            putchar(h[0]);
            putchar(h[1]);
        } else {
            putchar('0');
            putchar('0');
        }
    }
}

/* Выводит текстовое представление байт справа от HEX-дампа.
 * Непечатные символы заменяются на точку. */
static void print_ascii(const unsigned char *buf, int valid)
{
    int i;  /* индекс текущего байта в буфере */
    printf(" | ");
    for (i = 0; i < valid; i++) {
        if (is_printable(buf[i]))
            putchar((char)buf[i]);
        else
            putchar('.');
    }
}

/* Выводит смещение в виде 8 HEX-цифр с нулями слева и заглавными буквами */
static void print_offset(long pos)
{
    char digits[8];       /* массив из 8 символов для HEX-представления смещения */
    int i;                /* индекс для заполнения массива справа налево */
    unsigned long val = (unsigned long)pos;  /* беззнаковая копия для побитовых операций */
    for (i = 7; i >= 0; i--) {
        digits[i] = HEX_CHARS[val & 0x0F];
        val >>= 4;
    }
    for (i = 0; i < 8; i++)
        putchar(digits[i]);
}

/* Главная функция: читает файл блоками и выводит его в нужном формате */
void print_hex(FILE *fp, const Options *opts)
{
    int  group_size = opts->group_size;  /* размер одного кусочка в байтах (-g) */
    int  cols       = opts->cols;        /* сколько кусочков выводить в одной строке (-n) */
    long pos        = opts->offset;      /* текущая позиция в файле (начинается с -o) */
    long length     = opts->length;      /* сколько всего байт нужно вывести (-l), -1 = до конца */

    /* Сколько байт нужно прочитать, чтобы заполнить одну строку вывода */
    int row_bytes = group_size * cols;

    /* Буфер для хранения одного блока прочитанных данных */
    unsigned char *buf = (unsigned char *)malloc((size_t)row_bytes);
    if (!buf) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return;
    }

    /* Перематываем указатель чтения на нужное смещение */
    if (fseek(fp, pos, SEEK_SET) != 0) {
        fprintf(stderr, "Error: cannot seek to offset %ld\n", pos);
        free(buf);
        return;
    }

    long written = 0;  /* сколько байт уже выведено (для сравнения с length) */

    while (1) {
        int to_read = row_bytes;  /* сколько байт планируем прочитать в этом цикле */

        /* Если задано ограничение по длине, проверяем, сколько осталось прочитать */
        if (length >= 0) {
            long remaining = length - written;  /* сколько байт ещё осталось вывести */
            if (remaining <= 0) break;
            if (remaining < to_read) to_read = (int)remaining;
        }

        /* Сколько байт реально удалось прочитать из файла */
        int valid = (int)fread(buf, 1, (size_t)to_read, fp);
        if (valid == 0) break;

        /* Выводим смещение и два пробела */
        print_offset(pos);
        printf("  ");

        /* Сколько групп (полных и неполных) получилось в этой строке */
        int ngroups = valid / group_size + (valid % group_size != 0 ? 1 : 0);

        int g;  /* номер текущей группы (колонки) в строке */
        for (g = 0; g < cols; g++) {
            int byte_pos = g * group_size;  /* позиция первого байта текущей группы в буфере */
            if (byte_pos < valid) {
                print_group(buf, byte_pos, valid, group_size);
                /* Ставим пробел между группами */
                if (g < cols - 1) {
                    if (group_size == 1 || g < ngroups - 1)
                        putchar(' ');
                }
            } else if (group_size == 1) {
                /* Если группа равна 1 байту, выравниваем пустые места пробелами */
                putchar(' ');
                putchar(' ');
                if (g < cols - 1) putchar(' ');
            }
        }

        /* Текстовое представление выводится только если размер группы = 1 байт */
        if (group_size == 1)
            print_ascii(buf, valid);

        putchar('\n');

        written += valid;  /* увеличиваем счётчик выведенных байт */
        pos     += valid;  /* сдвигаем текущую позицию в файле */

        /* Если прочитали меньше, чем просили — файл закончился */
        if (valid < to_read) break;
    }

    free(buf);
}

/* Открывает один файл и передает его в функцию печати */
int print_file(const char *path, const Options *opts)
{
    FILE *fp = fopen(path, "rb");  /* открываем файл в двоичном режиме для чтения */
    if (!fp) {
        fprintf(stderr, "Error: cannot open file '%s'\n", path);
        return -1;
    }
    print_hex(fp, opts);
    fclose(fp);
    return 0;
}

/* Читает все файлы в указанной папке и выводит их по очереди.
 * Служебные папки "." и ".." игнорируются. */
int print_dir(const char *dir_path, const Options *opts)
{
    DIR *dir = opendir(dir_path);  
    if (!dir) {
        fprintf(stderr, "Error: cannot open directory '%s'\n", dir_path);
        return -1;
    }

    struct dirent *entry;  /* текущая запись (файл или папка) при обходе директории */
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        /* Собираем полный путь к файлу: "папка/имя_файла" */
        size_t len = strlen(dir_path) + 1 + strlen(entry->d_name) + 1;  /* длина строки пути + 1 для \0 */
        char *fpath = (char *)malloc(len);  /* буфер для полного пути */
        if (!fpath) {
            fprintf(stderr, "Error: memory allocation failed\n");
            continue;
        }
        snprintf(fpath, len, "%s/%s", dir_path, entry->d_name);

        /* Проверяем, что это именно обычный файл, а не подпапка */
        struct stat st;  /* структура с информацией о файле (размер, тип и т.д.) */
        if (stat(fpath, &st) == 0 && S_ISREG(st.st_mode)) {
            printf("=== %s ===\n", fpath);
            print_file(fpath, opts);
        }

        free(fpath);
    }

    closedir(dir);
    return 0;
}