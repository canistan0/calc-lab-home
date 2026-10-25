# calc-lab-home
Arithmetic parser in C --- graded laboratory assignment

## About the project

This program reads arithmetic expressions from text files. It calculates each answer and saves the results in an output file.

If the program finds an expression error, it saves only `ERROR:position`. The position shows where the first error happened.

## Features

- Integer and decimal numbers
- Addition, subtraction, multiplication, and division
- Powers using `**`
- Parentheses and the correct calculation order
- Blank lines and comment lines starting with `#`
- One input file or all `.txt` files directly inside a folder
- Automatic output folder creation

Unary operators are not allowed. For example, write `(0 - 5)` instead of `-5`.

Integer division removes the decimal part. For example, `7 / 2` gives `3`, but `7 / 2.0` gives `3.5`.

## Compile

Run this command in the terminal:

```bash
gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
```

## Run the program

Process one file:

```bash
./calc -o results practice.txt
```

Process a folder:

```bash
./calc -d demo_inputs -o results
```

Use the default output folder:

```bash
./calc practice.txt
```

The program also supports `--dir` and `--output-dir`.

For `practice.txt`, the output filename is:

```text
practice_yabesh_canistan_raj_kumar_231ADB199.txt
```

## Tests

Extract the supplied `calc-tests.zip`, then run:

```bash
sh calc-tests/run_tests.sh ./calc
sh calc-tests/cli.sh ./calc
```

My test run passed all 44 expression tests and all 12 command-line tests.

To check for memory and undefined-behaviour errors, compile with:

```bash
gcc -O1 -g -Wall -Wextra -std=c17 -fsanitize=address,undefined -o calc calc.c -lm
```

Then run both test scripts again.

## Resources 

I used the assignment instructions, supplied tests, and Google.
GNU C Library Manual, "Parsing of Floats".
Theodore Norvell, "Parsing Expressions by Recursive Descent".
GNU C Library Manual, "Changing the Size of a Block".