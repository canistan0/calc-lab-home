// Yabesh Canistan Raj Kumar 231ADB199
// https://github.com/canistan0/calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>

// These values are used in output filenames.
// Use lowercase letters for the name and surname.
#define STUDENT_NAME "yabesh_canistan"
#define STUDENT_LASTNAME "raj_kumar"
#define STUDENT_ID "231ADB199"

typedef struct {
    int is_float;
    long long integer;
    double real;
} Value;

typedef enum {
    TOK_NUMBER,TOK_PLUS,TOK_MINUS,TOK_STAR,TOK_SLASH,TOK_POWER,TOK_LPAREN,TOK_RPAREN,TOK_NEWLINE,TOK_INVALID,TOK_END
} TokenType;

typedef struct {
    TokenType type;
    Value value;
    size_t position;
} Token;

typedef struct {
    FILE *file;
    int current;
    size_t position;
    int line_has_token;
    int memory_error;
} Lexer;

typedef struct {
    Lexer lexer;
    Token token;
    size_t error_position;
} Parser;

// The position starts at 1. Every character counts, including newlines.
// EOF keeps the position just after the last character.
static void advance(Lexer *lexer)
{
    if (lexer->current != EOF){
        lexer->current = fgetc(lexer->file);
        lexer->position++;}
}

static Token next_token(Lexer *lexer)
{
    // Spaces and tabs count towards positions, but are not tokens.
    while (lexer->current == ' ' || lexer->current == '\t'){
        advance(lexer);}
    // A comment is allowed only before the first token on a line.
    // advance still counts every character in the comment.
    if (lexer->current == '#' && !lexer->line_has_token) {
        while (lexer->current != '\n' && lexer->current != EOF) {
            advance(lexer);
        }
    }

    Token token = {0};
    token.type = TOK_INVALID;
    token.position = lexer->position;

    if (lexer->current == EOF){
        token.type = TOK_END;
        return token;}
        // A newline starts a fresh line. Spaces do not mark a line as used.
    if (lexer->current == '\n') {
        lexer->line_has_token = 0;
    } else {
        lexer->line_has_token = 1;
    }

    if (lexer->current >= '0' && lexer->current <= '9'){
        token.type = TOK_NUMBER;

        // Each new digit moves the previous digits one place to the left.
        // The task says integer values in the tests do not overflow.
        while (lexer->current >= '0' && lexer->current <= '9'){
            token.value.integer = token.value.integer * 10 + (lexer->current - '0');
            advance(lexer);}

        return token;
        
    }

    int symbol = lexer->current;
    advance(lexer);

    switch (symbol){
        case '+':
            token.type = TOK_PLUS;
            break;
        case '-':
            token.type = TOK_MINUS;
            break;
        case '*':
            // Two stars together form one power operator.
            if (lexer->current == '*') {
                advance(lexer);
                token.type = TOK_POWER;
            } else {
                token.type = TOK_STAR;
            }
            break;
        case '/':
            token.type = TOK_SLASH;
            break;
        case '(':
            token.type = TOK_LPAREN;
            break;
        case ')':
            token.type = TOK_RPAREN;
            break;
        case '\n':
            token.type = TOK_NEWLINE;
            break;
        default:
            token.type = TOK_INVALID;
            break;
    }
    return token;
}


static void move_token(Parser *parser)
{
    parser->token = next_token(&parser->lexer);}

static void set_error(Parser *parser, size_t position)
{
    // I keep the first error because the task asks for only one error.
    if (parser->error_position == 0) {
        parser->error_position = position;}
}

static double as_double(Value value)
{
    return value.is_float ? value.real : (double)value.integer;
}

static Value calculate(Parser *parser, Value left, Value right,
                       TokenType operation, size_t operator_position,
                       size_t right_position)
{
    Value result = {0};

    if (operation == TOK_SLASH &&
        (right.is_float ? right.real == 0.0 : right.integer == 0)) {
        // Division by zero points to the start of the right operand.
        set_error(parser, right_position);
        return result;
    }

    // If either operand is a float, I use double arithmetic.
    if (left.is_float || right.is_float) {
        double a = as_double(left);
        double b = as_double(right);
        result.is_float = 1;

        switch (operation) {
            case TOK_PLUS:
                result.real = a + b;
                break;
            case TOK_MINUS:
                result.real = a - b;
                break;
            case TOK_STAR:
                result.real = a * b;
                break;
            case TOK_SLASH:
                result.real = a / b;
                break;
            case TOK_POWER:
                result.real = pow(a, b);
                break;
            default:
                break;
        }

        // Infinity and NaN are errors at the operator.
        if (!isfinite(result.real)) {
            set_error(parser, operator_position);
        }

        return result;
    }

    // Integer operations stay separate to keep exact values and division.
    // The task says the integer tests do not overflow.
    switch (operation) {
        case TOK_PLUS:
            result.integer = left.integer + right.integer;
            break;
        case TOK_MINUS:
            result.integer = left.integer - right.integer;
            break;
        case TOK_STAR:
            result.integer = left.integer * right.integer;
            break;
        case TOK_SLASH:
            result.integer = left.integer / right.integer;
            break;
        case TOK_POWER: {
            if (right.integer < 0) {
                set_error(parser, right_position);
                return result;
            }

            long long base = left.integer;
            long long exponent = right.integer;
            result.integer = 1;

            // Squaring needs fewer steps than multiplying many times.
            // Starting at 1 also handles 0 ** 0.
            while (exponent > 0) {
                if (exponent % 2 != 0) {
                    result.integer *= base;
                }

                exponent /= 2;

                if (exponent > 0) {
                    base *= base;
                }
            }
            break;
        }
        default:
            break;
    }

    return result;
}

static Value read_expression(Parser *parser);

static Value read_primary(Parser *parser)
{
    Value result = {0};

    if (parser->token.type == TOK_NUMBER) {
        result = parser->token.value;
        move_token(parser);
        return result;
    }

    if (parser->token.type == TOK_LPAREN) {
        move_token(parser);
        result = read_expression(parser);

        if (parser->error_position != 0) {
            return result;
        }

        if (parser->token.type != TOK_RPAREN) {
            // This is where the closing parenthesis was needed.
            set_error(parser, parser->token.position);
            return result;
        }

        move_token(parser);
        return result;
    }

    // A number or opening parenthesis is needed here.
    // This also rejects unary signs and empty parentheses.
    set_error(parser, parser->token.position);
    return result;
}

static Value read_power(Parser *parser)
{
    Value left = read_primary(parser);

    if (parser->error_position == 0 &&
        parser->token.type == TOK_POWER) {
        size_t operator_position = parser->token.position;
        move_token(parser);
        size_t right_position = parser->token.position;

        // Calling read_power again gives 2 ** (3 ** 2).
        Value right = read_power(parser);

        if (parser->error_position == 0) {
            left = calculate(parser, left, right, TOK_POWER,
                             operator_position, right_position);
        }
    }

    return left;
}

static Value read_term(Parser *parser)
{
    Value left = read_power(parser);

    // Powers finish first. Multiplication and division run left to right.
    while (parser->error_position == 0 &&
           (parser->token.type == TOK_STAR ||
            parser->token.type == TOK_SLASH)) {
        TokenType operation = parser->token.type;
        size_t operator_position = parser->token.position;
        move_token(parser);
        size_t right_position = parser->token.position;

        Value right = read_power(parser);

        if (parser->error_position == 0) {
            left = calculate(parser, left, right, operation,
                             operator_position, right_position);
        }
    }

    return left;
}

static Value read_expression(Parser *parser)
{
    Value left = read_term(parser);

    // Terms finish first, so * and / have higher priority than + and -.
    while (parser->error_position == 0 &&
           (parser->token.type == TOK_PLUS ||
            parser->token.type == TOK_MINUS)) {
        TokenType operation = parser->token.type;
        size_t operator_position = parser->token.position;
        move_token(parser);
        size_t right_position = parser->token.position;

        Value right = read_term(parser);

        if (parser->error_position == 0) {
            left = calculate(parser, left, right, operation,
                             operator_position, right_position);
        }
    }

    return left;
}

static int evaluate_file(const char *input_path, FILE *output)
{
    FILE *input = fopen(input_path, "r");
    if (input == NULL) {
        perror("Cannot open input file");
        return 1;
    }

    // I save results here first. A later error must remove all results.
    // A temporary file also avoids keeping many results in memory.
    FILE *results = tmpfile();

    if (results == NULL) {
        perror("Cannot create temporary file");
        fclose(input);
        return 1;
    }

    Parser parser = {0};
    parser.lexer.file = input;
    parser.lexer.position = 1;
    parser.lexer.current = fgetc(input);
    move_token(&parser);

    int io_error = 0;

    while (parser.token.type != TOK_END &&
           parser.error_position == 0) {
        if (parser.token.type == TOK_NEWLINE) {
            move_token(&parser);
            continue;
        }

        Value result = read_expression(&parser);
        if (parser.error_position != 0) {
            break;
        }
        

        // After an expression, only a newline or end of file is allowed.
        if (parser.token.type != TOK_NEWLINE &&
            parser.token.type != TOK_END) {
            set_error(&parser, parser.token.position);
            break;
        }
        // The task asks for 15 significant digits when printing floats.
        int written;

        if (result.is_float) {
        written = fprintf(results, "%.15g\n", result.real);
        } else {
            written = fprintf(results, "%lld\n", result.integer);
            }

        if (written < 0) {
        io_error = 1;
        break;
        }



        if (parser.token.type == TOK_NEWLINE) {
            move_token(&parser);
        }
    }

    if (ferror(input)) {
        io_error = 1;
    }

    if (fclose(input) == EOF) {
        io_error = 1;
    }

    if (!io_error) {
        if (parser.error_position != 0) {
            // Expression errors are normal results, so they return 0.
            if (fprintf(output, "ERROR:%zu\n", parser.error_position) < 0) {
                io_error = 1;
            }
        } else {
            if (fflush(results) == EOF ||
                fseek(results, 0, SEEK_SET) != 0) {
                io_error = 1;
            } else {
                int ch;

                while ((ch = fgetc(results)) != EOF) {
                    if (fputc(ch, output) == EOF) {
                        io_error = 1;
                        break;
                    }
                }

                if (ferror(results)) {
                    io_error = 1;
                }
            }
        }
    }

    if (fclose(results) == EOF) {
        io_error = 1;
    }

    if (fflush(output) == EOF) {
        io_error = 1;
    }

    if (io_error) {
        fprintf(stderr, "Cannot read or write file data\n");
        return 1;
    }
    return 0;
}

static int process_one(const char *input_path, const char *output_dir)
{
    const char *base = strrchr(input_path, '/');
    base = base == NULL ? input_path : base + 1;

    size_t base_length = strlen(base);

    if (base_length >= 4 &&
        strcmp(base + base_length - 4, ".txt") == 0) {
        base_length -= 4;
    }

    size_t dir_length = strlen(output_dir);
    size_t path_size = dir_length + base_length
                     + strlen(STUDENT_NAME) + strlen(STUDENT_LASTNAME)
                     + strlen(STUDENT_ID) + 9;

    char *output_path = malloc(path_size);

    if (output_path == NULL) {
        fprintf(stderr, "Not enough memory for output path\n");
        return 1;
    }

    memcpy(output_path, output_dir, dir_length);
    output_path[dir_length] = '/';
    memcpy(output_path + dir_length + 1, base, base_length);

    size_t prefix_length = dir_length + 1 + base_length;

    snprintf(output_path + prefix_length,
             path_size - prefix_length,
             "_%s_%s_%s.txt",
             STUDENT_NAME, STUDENT_LASTNAME, STUDENT_ID);

    FILE *output = fopen(output_path, "w");

    if (output == NULL) {
        perror("Cannot open output file");
        free(output_path);
        return 1;
    }

    int status = evaluate_file(input_path, output);

    if (fclose(output) == EOF) {
        fprintf(stderr, "Cannot close output file\n");
        status = 1;
    }

    free(output_path);
    return status;
}

static int process_directory(const char *input_dir,
                             const char *output_dir)
{
    DIR *directory = opendir(input_dir);

    if (directory == NULL) {
        perror("Cannot open input directory");
        return 1;
    }

    int status = 0;

    for (;;) {
        // errno helps separate the end of the folder from a read error.
        errno = 0;
        struct dirent *entry = readdir(directory);

        if (entry == NULL) {
            if (errno != 0) {
                perror("Cannot read input directory");
                status = 1;
            }
            break;
        }

        size_t name_length = strlen(entry->d_name);

        // Only names ending in .txt are included.
        if (name_length < 4 ||
            strcmp(entry->d_name + name_length - 4, ".txt") != 0) {
            continue;
        }

        size_t path_size = strlen(input_dir) + name_length + 2;
        char *input_path = malloc(path_size);

        if (input_path == NULL) {
            fprintf(stderr, "Not enough memory for input path\n");
            status = 1;
            break;
        }

        snprintf(input_path, path_size, "%s/%s",
                 input_dir, entry->d_name);

        struct stat info;

        if (stat(input_path, &info) != 0) {
            perror("Cannot access directory entry");
            free(input_path);
            status = 1;
            break;
        }

        // A folder called something.txt is still a folder, so I skip it.
        // I do not enter subfolders.
        if (S_ISREG(info.st_mode)) {
            status = process_one(input_path, output_dir);
        }

        free(input_path);

        if (status != 0) {
            break;
        }
    }

    if (closedir(directory) != 0) {
        perror("Cannot close input directory");
        status = 1;
    }

    return status;
}

int main(int argc, char *argv[])
{
    const char *input_file = NULL;
    const char *input_dir = NULL;
    const char *output_dir = NULL;
    char *default_dir = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 ||
            strcmp(argv[i], "--output-dir") == 0) {
            if (output_dir != NULL || i + 1 >= argc) {
                fprintf(stderr, "Give one output folder after -o\n");
                return 1;
            }

            output_dir = argv[++i];
        } else if (strcmp(argv[i], "-d") == 0 ||
                   strcmp(argv[i], "--dir") == 0) {
            if (input_dir != NULL || i + 1 >= argc) {
                fprintf(stderr, "Give one input folder after -d\n");
                return 1;
            }

            input_dir = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        } else {
            if (input_file != NULL) {
                fprintf(stderr, "Give only one input file\n");
                return 1;
            }

            input_file = argv[i];
        }
    }

    // There must be one input file or one input folder.
    if ((input_file == NULL && input_dir == NULL) ||
        (input_file != NULL && input_dir != NULL)) {
        fprintf(stderr,
                "Usage: %s [-d DIR] [-o OUTDIR] [input.txt]\n",
                argv[0]);
        return 1;
    }

    const char *source = input_dir != NULL ? input_dir : input_file;
    struct stat info;

    if (stat(source, &info) != 0) {
        perror("Cannot access input");
        return 1;
    }

    if (input_dir != NULL) {
        if (!S_ISDIR(info.st_mode)) {
            fprintf(stderr, "Directory input must be a folder\n");
            return 1;
        }
    } else if (!S_ISREG(info.st_mode)) {
        fprintf(stderr, "File input must be a regular file\n");
        return 1;
    }

    if (output_dir == NULL) {
        // I ignore trailing slashes when finding the folder name.
        size_t end = strlen(source);

        while (end > 1 && source[end - 1] == '/') {
            end--;
        }

        size_t start = end;

        while (start > 0 && source[start - 1] != '/') {
            start--;
        }

        size_t base_length = end - start;

        if (input_dir == NULL && base_length >= 4 &&
            memcmp(source + end - 4, ".txt", 4) == 0) {
            base_length -= 4;
        }

        size_t dir_size = base_length + strlen(STUDENT_NAME)
                        + strlen(STUDENT_ID) + 3;

        default_dir = malloc(dir_size);

        if (default_dir == NULL) {
            fprintf(stderr, "Not enough memory for output folder\n");
            return 1;
        }

        memcpy(default_dir, source + start, base_length);

        snprintf(default_dir + base_length,
                 dir_size - base_length,
                 "_%s_%s", STUDENT_NAME, STUDENT_ID);

        output_dir = default_dir;
    }

    if (stat(output_dir, &info) == 0) {
        if (!S_ISDIR(info.st_mode)) {
            fprintf(stderr, "Output path must be a directory\n");
            free(default_dir);
            return 1;
        }
    } else if (mkdir(output_dir, 0777) != 0) {
        perror("Cannot create output directory");
        free(default_dir);
        return 1;
    }

    int status;

    if (input_dir != NULL) {
        status = process_directory(input_dir, output_dir);
    } else {
        status = process_one(input_file, output_dir);
    }

    free(default_dir);
    return status;
}