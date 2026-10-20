// Yabesh Canistan Raj Kumar 231ADB199
// https://github.com/canistan0/calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm

#include <stdio.h>
int main(int argc, char *argv[])
{
    if (argc != 2){
        fprintf(stderr, "Usage: %s input.txt\n", argv[0]);
        return 1;}

    FILE *input = fopen(argv[1], "r");

    if (input == NULL){
        perror("Cannot open input file");
        return 1;}

    // I use int because fgetc returns a character or EOF (end of file).
    int ch;
    while ((ch = fgetc(input)) != EOF) {
        if (putchar(ch) == EOF) {
            fprintf(stderr, "Cannot write to standard output\n");
            fclose(input);
            return 1;}}

    if (ferror(input)){
        fprintf(stderr, "Cannot read input file\n");
        fclose(input);
        return 1;}

    if (fclose(input) == EOF){
        fprintf(stderr, "Cannot close input file\n");
        return 1;}
    return 0;
}
