// Yabesh Canistan Raj Kumar 231ADB199
// https://github.com/canistan0/calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm

#include <stdio.h>

typedef enum {
    TOK_NUMBER,TOK_PLUS,TOK_MINUS,TOK_STAR,TOK_SLASH,TOK_POWER,TOK_LPAREN,TOK_RPAREN,TOK_NEWLINE,TOK_INVALID,TOK_END
} TokenType;

typedef struct {
    TokenType type;
    long long value;
    size_t position;
} Token;

typedef struct {
    FILE *file;
    int current;
    size_t position;
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

    Token token = {TOK_INVALID, 0, lexer->position};

    if (lexer->current == EOF){
        token.type = TOK_END;
        return token;}

    if (lexer->current >= '0' && lexer->current <= '9'){
        token.type = TOK_NUMBER;

        // Each new digit moves the previous digits one place to the left.
        // The task says integer values in the tests do not overflow.
        while (lexer->current >= '0' && lexer->current <= '9'){
            token.value = token.value * 10 + (lexer->current - '0');
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

static long long read_number(Parser *parser)
{
    if (parser->token.type != TOK_NUMBER) {
        // This token is where the parser expected a number.
        set_error(parser, parser->token.position);
        return 0;
    }

    long long value = parser->token.value;
    move_token(parser);
    return value;
}

static long long read_expression(Parser *parser)
{
    long long result = read_number(parser);

    // The loop calculates + and - from left to right.
    while (parser->error_position == 0 &&
           (parser->token.type == TOK_PLUS ||
            parser->token.type == TOK_MINUS)) {
        TokenType operation = parser->token.type;
        move_token(parser);

        long long right = read_number(parser);

        if (parser->error_position != 0) {
            return 0;
        }

        if (operation == TOK_PLUS) {
            result += right;
        } else {
            result -= right;
        }
    }
    return result;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s input.txt\n", argv[0]);
        return 1;}

    FILE *input = fopen(argv[1], "r");

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

        long long result = read_expression(&parser);
        if (parser.error_position != 0) {
            break;
        }

        // After an expression, only a newline or end of file is allowed.
        if (parser.token.type != TOK_NEWLINE &&
            parser.token.type != TOK_END) {
            set_error(&parser, parser.token.position);
            break;
        }

        if (fprintf(results, "%lld\n", result) < 0) {
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
            if (printf("ERROR:%zu\n", parser.error_position) < 0) {
                io_error = 1;
            }
        } else {
            if (fflush(results) == EOF ||
                fseek(results, 0, SEEK_SET) != 0) {
                io_error = 1;
            } else {
                int ch;

                while ((ch = fgetc(results)) != EOF) {
                    if (putchar(ch) == EOF) {
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

    if (fflush(stdout) == EOF) {
        io_error = 1;
    }

    if (io_error) {
        fprintf(stderr, "Cannot read or write file data\n");
        return 1;
    }
    return 0;
}