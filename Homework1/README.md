# Homework 1

This directory contains the code for the first homework problem set. The due
date for this assignment is March 4, 2026 at 11:59 p.m. CDT.

Where requested by the homework, analyses and comparisons between user-generated
code and LLM-generated code is included at the top of the corresponding file.

## Naming Schema

Files are named according to which problem of the homework they relate to. For
example, question 3 of this homework set has [problem3.c](./problem3.c),
[problem3.h](./problem3.h), and [problem3main.c](./problem3main.c). The main
driver code is found in `problem3main.c`, and the supporting code (function and
struct definitions, constants, etc.) are found in `problem3.c` and `problem3.h`.

If a problem has only a single `problem#main.c`, then no additional code outside
that file is used for that problem.

A file with a `g` before the number is a problem in the graduate section of the
homework. This directory has `problemg1` for the Game of Life simulation that
was part of the graduate level.

## Additional Files

Two additional files, [Terminal.c](./Terminal.c) and [Terminal.h](./Terminal.h)
are included for terminal formatting. These contain functions for a basic
library I wrote that uses ANSI escape code to format terminal output. They're
used exclusively for the graduate question of this assignment when printing
out the world for the Game of Life.

## Compiling

Compiling uses the `make` utility. Issuing just `make` without specifying a
target compiles every problem's code. The resulting executable is simply name
`problem#` with no extension.

## Running

There are built in run targets for every problem in this set. These targets have
names in the form `runp#`. The command that is issued from this target is
typically a `mpirun` command with a predetermined value for the `-np` argument.

## Example Compiling and Running

For more information on how compiling and running is handled for each target
see the included [Makefile](./Makefile).

```sh
make clean
make
make runp1

make clean
make problemg1
make runpg1
```
