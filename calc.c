// Palla Sanjana Grace 241ADB116
// https://github.com/SanjanaGrace/Programming_Languages_C_2026_Fall_Labs
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: C17 standard library documentation; OpenAI ChatGPT, prompt
// "Help me implement the graded arithmetic parser in C from the supplied
// specification, including exact error positions and command-line behavior."
// The design and comments below explain the decisions used in this program.

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define STUDENT_NAME "palla"
#define STUDENT_LASTNAME "sanjana_grace"
#define STUDENT_ID "241ADB116"

/* A value remembers its arithmetic kind so mixed expressions use double. */
typedef struct {
    int is_float;
    long long i;
    double d;
} Value;

typedef struct {
    const char *text;
    size_t length;
    size_t index;
    size_t error;
    int failed;
} Parser;

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} Buffer;

static void set_error(Parser *p, size_t position)
{
    if (!p->failed) {
        p->failed = 1;
        p->error = position;
    }
}

static size_t next_nonspace(const Parser *p)
{
    size_t n = p->index;
    while (n < p->length && (p->text[n] == ' ' || p->text[n] == '\t')) {
        n++;
    }
    return n;
}

static void skip_spaces(Parser *p)
{
    p->index = next_nonspace(p);
}

static Value integer_value(long long value)
{
    Value result = {0, value, (double)value};
    return result;
}

static Value float_value(double value)
{
    Value result = {1, 0, value};
    return result;
}

static Value parse_expression(Parser *p);
static Value parse_term(Parser *p);
static Value parse_power(Parser *p);
static Value parse_primary(Parser *p);

/*
 * Reading the entire file once and doubling capacity avoids repeated copying
 * for multi-megabyte inputs. Positions remain offsets into this unchanged
 * buffer, so newlines and comment characters count naturally.
 */
static int read_file(const char *path, Buffer *buffer)
{
    FILE *file = fopen(path, "rb");
    int ch;
    if (file == NULL) {
        return 0;
    }
    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
    while ((ch = fgetc(file)) != EOF) {
        if (buffer->length == buffer->capacity) {
            size_t new_capacity = buffer->capacity == 0 ? 8192 : buffer->capacity * 2;
            char *new_data = realloc(buffer->data, new_capacity);
            if (new_data == NULL) {
                free(buffer->data);
                fclose(file);
                return 0;
            }
            buffer->data = new_data;
            buffer->capacity = new_capacity;
        }
        buffer->data[buffer->length++] = (char)ch;
    }
    if (ferror(file)) {
        free(buffer->data);
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

static Value parse_primary(Parser *p)
{
    size_t start;
    if (p->failed) {
        return integer_value(0);
    }
    skip_spaces(p);
    start = p->index;
    if (start >= p->length || p->text[start] == '\n') {
        /* The missing operand is reported at the newline or virtual EOF. */
        set_error(p, start + 1);
        return integer_value(0);
    }
    if (p->text[start] == '(') {
        Value inside;
        p->index++;
        inside = parse_expression(p);
        if (p->failed) {
            return integer_value(0);
        }
        skip_spaces(p);
        if (p->index >= p->length || p->text[p->index] == '\n') {
            /* A closing parenthesis missing at line end is reported there. */
            set_error(p, p->index + 1);
            return integer_value(0);
        }
        if (p->text[p->index] != ')') {
            set_error(p, p->index + 1);
            return integer_value(0);
        }
        p->index++;
        return inside;
    }
    if (!isdigit((unsigned char)p->text[start])) {
        set_error(p, start + 1);
        return integer_value(0);
    }

    {
        size_t digits_end = start;
        int has_dot = 0;
        while (digits_end < p->length && isdigit((unsigned char)p->text[digits_end])) {
            digits_end++;
        }
        if (digits_end < p->length && p->text[digits_end] == '.') {
            size_t fraction_start = digits_end + 1;
            has_dot = 1;
            digits_end = fraction_start;
            while (digits_end < p->length && isdigit((unsigned char)p->text[digits_end])) {
                digits_end++;
            }
            if (digits_end == fraction_start) {
                /* This catches both 5. and the dot's exact position. */
                set_error(p, fraction_start);
                return integer_value(0);
            }
        }
        if (digits_end < p->length && (p->text[digits_end] == 'e' || p->text[digits_end] == 'E')) {
            set_error(p, digits_end + 1);
            return integer_value(0);
        }
        {
            size_t count = digits_end - start;
            char *number = malloc(count + 1);
            char *endptr;
            if (number == NULL) {
                set_error(p, start + 1);
                return integer_value(0);
            }
            memcpy(number, p->text + start, count);
            number[count] = '\0';
            p->index = digits_end;
            if (has_dot) {
                double d = strtod(number, &endptr);
                if (*endptr != '\0') {
                    free(number);
                    set_error(p, start + 1);
                    return integer_value(0);
                }
                free(number);
                return float_value(d);
            } else {
                long long i = strtoll(number, &endptr, 10);
                if (*endptr != '\0') {
                    free(number);
                    set_error(p, start + 1);
                    return integer_value(0);
                }
                free(number);
                return integer_value(i);
            }
        }
    }
}

/*
 * Power recurses on the right operand, which makes ** right-associative:
 * 2 ** 3 ** 2 becomes 2 ** (3 ** 2), unlike the left-folding term loop.
 */
static Value parse_power(Parser *p)
{
    Value left = parse_primary(p);
    if (p->failed) {
        return integer_value(0);
    }
    skip_spaces(p);
    if (p->index + 1 < p->length && p->text[p->index] == '*' && p->text[p->index + 1] == '*') {
        size_t operator_position = p->index + 1;
        Value right;
        p->index += 2;
        right = parse_power(p);
        if (p->failed) {
            return integer_value(0);
        }
        if (!right.is_float && right.i < 0) {
            /* The exponent token starts after the ** and skipped spaces. */
            size_t exponent_start = operator_position + 2;
            while (exponent_start <= p->length && exponent_start > 0 &&
                   (p->text[exponent_start - 1] == ' ' || p->text[exponent_start - 1] == '\t')) {
                exponent_start--;
            }
            /* Recompute from the operator's right side to exclude spaces. */
            exponent_start = operator_position + 1;
            while (exponent_start < p->length &&
                   (p->text[exponent_start] == ' ' || p->text[exponent_start] == '\t')) {
                exponent_start++;
            }
            set_error(p, exponent_start + 1);
            return integer_value(0);
        }
        if (!left.is_float && !right.is_float) {
            long long exponent = right.i;
            long long result = 1;
            long long base = left.i;
            while (exponent > 0) {
                if (exponent % 2 != 0) {
                    result *= base;
                }
                exponent /= 2;
                if (exponent > 0) {
                    base *= base;
                }
            }
            return integer_value(result);
        } else {
            double result = pow(left.is_float ? left.d : (double)left.i,
                                right.is_float ? right.d : (double)right.i);
            if (!isfinite(result)) {
                set_error(p, operator_position);
                return integer_value(0);
            }
            return float_value(result);
        }
    }
    return left;
}

static Value parse_term(Parser *p)
{
    Value left = parse_power(p);
    while (!p->failed) {
        size_t op;
        skip_spaces(p);
        op = p->index;
        if (op >= p->length || p->text[op] == '\n' || p->text[op] == '+' || p->text[op] == '-' || p->text[op] == ')') {
            break;
        }
        if (p->text[op] != '*' && p->text[op] != '/') {
            break;
        }
        if (p->text[op] == '*' && op + 1 < p->length && p->text[op + 1] == '*') {
            break;
        }
        p->index++;
        {
            size_t right_start = next_nonspace(p);
            Value right;
            p->index = right_start;
            right = parse_power(p);
            if (p->failed) {
                return integer_value(0);
            }
            if (p->text[op] == '/' && ((right.is_float && right.d == 0.0) || (!right.is_float && right.i == 0))) {
                /* Division-by-zero points to the start of its right operand. */
                set_error(p, right_start + 1);
                return integer_value(0);
            }
            if (!left.is_float && !right.is_float) {
                left = p->text[op] == '*' ? integer_value(left.i * right.i) : integer_value(left.i / right.i);
            } else {
                double a = left.is_float ? left.d : (double)left.i;
                double b = right.is_float ? right.d : (double)right.i;
                double result = p->text[op] == '*' ? a * b : a / b;
                if (!isfinite(result)) {
                    set_error(p, op + 1);
                    return integer_value(0);
                }
                left = float_value(result);
            }
        }
    }
    return left;
}

static Value parse_expression(Parser *p)
{
    Value left = parse_term(p);
    while (!p->failed) {
        size_t op;
        skip_spaces(p);
        op = p->index;
        if (op >= p->length || p->text[op] == '\n' || p->text[op] == ')') {
            break;
        }
        if (p->text[op] != '+' && p->text[op] != '-') {
            break;
        }
        p->index++;
        {
            Value right = parse_term(p);
            if (p->failed) {
                return integer_value(0);
            }
            if (!left.is_float && !right.is_float) {
                left = p->text[op] == '+' ? integer_value(left.i + right.i) : integer_value(left.i - right.i);
            } else {
                double a = left.is_float ? left.d : (double)left.i;
                double b = right.is_float ? right.d : (double)right.i;
                double result = p->text[op] == '+' ? a + b : a - b;
                if (!isfinite(result)) {
                    set_error(p, op + 1);
                    return integer_value(0);
                }
                left = float_value(result);
            }
        }
    }
    return left;
}

static int evaluate(const Buffer *buffer, FILE *output, size_t *error_position)
{
    Parser p = {buffer->data, buffer->length, 0, 0, 0};
    while (p.index < p.length) {
        size_t line_start = p.index;
        size_t first = p.index;
        while (first < p.length && (p.text[first] == ' ' || p.text[first] == '\t')) {
            first++;
        }
        if (first >= p.length) {
            break;
        }
        if (p.text[first] == '\n') {
            p.index = first + 1;
            continue;
        }
        if (p.text[first] == '#') {
            while (p.index < p.length && p.text[p.index] != '\n') {
                p.index++;
            }
            if (p.index < p.length) {
                p.index++;
            }
            continue;
        }
        p.index = line_start;
        {
            Value result = parse_expression(&p);
            if (p.failed) {
                *error_position = p.error;
                return 0;
            }
            skip_spaces(&p);
            if (p.index < p.length && p.text[p.index] != '\n') {
                set_error(&p, p.index + 1);
                *error_position = p.error;
                return 0;
            }
            if (result.is_float) {
                fprintf(output, "%.15g\n", result.d);
            } else {
                fprintf(output, "%lld\n", result.i);
            }
        }
        if (p.index < p.length && p.text[p.index] == '\n') {
            p.index++;
        }
    }
    return 1;
}

static const char *base_name(const char *path)
{
    const char *slash = strrchr(path, '/');
    return slash == NULL ? path : slash + 1;
}

static void remove_txt(char *name)
{
    size_t length = strlen(name);
    if (length >= 4 && strcmp(name + length - 4, ".txt") == 0) {
        name[length - 4] = '\0';
    }
}

static int make_output_path(const char *directory, const char *input, char *path, size_t path_size)
{
    const char *base = base_name(input);
    char stem[512];
    int written;
    if (strlen(base) >= sizeof(stem)) {
        return 0;
    }
    strcpy(stem, base);
    remove_txt(stem);
    written = snprintf(path, path_size, "%s/%s_%s_%s_%s.txt", directory, stem,
                       STUDENT_NAME, STUDENT_LASTNAME, STUDENT_ID);
    return written >= 0 && (size_t)written < path_size;
}

static int ensure_directory(const char *path)
{
    struct stat info;
    if (stat(path, &info) == 0) {
        return S_ISDIR(info.st_mode);
    }
    if (errno != ENOENT) {
        return 0;
    }
    return mkdir(path, 0777) == 0;
}

static int process_one(const char *input, const char *output_directory)
{
    Buffer buffer;
    char output_path[1024];
    FILE *output;
    FILE *scratch;
    size_t error_position = 0;
    int ch;
    int ok;
    if (!read_file(input, &buffer)) {
        fprintf(stderr, "cannot read input file: %s\n", input);
        return 0;
    }
    if (!make_output_path(output_directory, input, output_path, sizeof(output_path))) {
        free(buffer.data);
        fprintf(stderr, "output path is too long\n");
        return 0;
    }
    scratch = tmpfile();
    if (scratch == NULL) {
        free(buffer.data);
        fprintf(stderr, "cannot create temporary output\n");
        return 0;
    }
    ok = evaluate(&buffer, scratch, &error_position);
    output = fopen(output_path, "wb");
    if (output == NULL) {
        free(buffer.data);
        fprintf(stderr, "cannot write output file: %s\n", output_path);
        return 0;
    }
    if (!ok) {
        fprintf(output, "ERROR:%zu\n", error_position);
    } else {
        rewind(scratch);
        while ((ch = fgetc(scratch)) != EOF) {
            fputc(ch, output);
        }
    }
    /* A written ERROR:<position> is a valid result, not a program failure. */
    ok = 1;
    fclose(scratch);
    if (fclose(output) != 0) {
        ok = 0;
    }
    free(buffer.data);
    return ok;
}

int main(int argc, char **argv)
{
    const char *input = NULL;
    const char *directory = NULL;
    const char *output_directory = NULL;
    int i;
    if (argc < 2) {
        fprintf(stderr, "usage: calc [-d DIR] [-o OUTDIR] [input.txt]\n");
        return 1;
    }
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) {
            if (i + 1 >= argc || directory != NULL || input != NULL) {
                fprintf(stderr, "invalid directory arguments\n");
                return 1;
            }
            directory = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output-dir") == 0) {
            if (i + 1 >= argc || output_directory != NULL) {
                fprintf(stderr, "invalid output arguments\n");
                return 1;
            }
            output_directory = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            return 1;
        } else if (input == NULL) {
            input = argv[i];
        } else {
            fprintf(stderr, "too many input paths\n");
            return 1;
        }
    }
    if ((directory == NULL && input == NULL) || (directory != NULL && input != NULL)) {
        fprintf(stderr, "provide either one input file or -d DIR\n");
        return 1;
    }
    if (output_directory == NULL) {
        char folder[1024];
        const char *source = directory != NULL ? directory : input;
        const char *base = base_name(source);
        char stem[512];
        if (strlen(base) >= sizeof(stem)) {
            fprintf(stderr, "input path is too long\n");
            return 1;
        }
        strcpy(stem, base);
        if (directory == NULL) {
            remove_txt(stem);
            if (snprintf(folder, sizeof(folder), "%s_%s_%s", stem, STUDENT_NAME, STUDENT_ID) >= (int)sizeof(folder)) {
                fprintf(stderr, "output path is too long\n");
                return 1;
            }
        } else if (snprintf(folder, sizeof(folder), "%s_%s_%s", stem, STUDENT_NAME, STUDENT_ID) >= (int)sizeof(folder)) {
            fprintf(stderr, "output path is too long\n");
            return 1;
        }
        output_directory = folder;
        if (!ensure_directory(output_directory)) {
            fprintf(stderr, "cannot create output directory: %s\n", output_directory);
            return 1;
        }
    } else if (!ensure_directory(output_directory)) {
        fprintf(stderr, "cannot create or use output directory: %s\n", output_directory);
        return 1;
    }

    if (directory != NULL) {
        DIR *dir = opendir(directory);
        struct dirent *entry;
        if (dir == NULL) {
            fprintf(stderr, "cannot open input directory: %s\n", directory);
            return 1;
        }
        while ((entry = readdir(dir)) != NULL) {
            size_t length = strlen(entry->d_name);
            char path[1024];
            struct stat info;
            if (length < 4 || strcmp(entry->d_name + length - 4, ".txt") != 0) {
                continue;
            }
            if (snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name) >= (int)sizeof(path) ||
                stat(path, &info) != 0 || !S_ISREG(info.st_mode)) {
                continue;
            }
            if (!process_one(path, output_directory)) {
                closedir(dir);
                return 1;
            }
        }
        closedir(dir);
    } else if (!process_one(input, output_directory)) {
        return 1;
    }
    return 0;
}
