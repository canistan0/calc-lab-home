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
