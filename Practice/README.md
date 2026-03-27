# Practice

This directory contains practice code for working with the various parallel frameworks. Each file contains a toy problem implemented using one or more of the programming models taught in the course.

# Naming Schema

File names follow a multi-sectional form where the first portion is the framework used to implement the problem and the second portion identifies the task for which the code is designed. For example, a program written in MPI that's designed to add numbers in parallel would have the name [MPI_ParallelAdd.c](./MPI_ParallelAdd.c) or something similar.

# Compiling and Running

Compiling utilizes the `make` utility and follows GNUMake syntax. Targets are named in a similar fashion to the file names. This means a file can be compiled simply by omitting the `.c`/`.cpp` extension of the file.

Additionally, a compiled program can be ran by including `Run_` before the compilation target. This will can the necessary executor if the framework used needs it (_e.g._ MPI needs `mpirun`).

An example of compiling and running the `MPI_ParallelAdd` program can be found below.

```sh
make MPI_ParallelAdd
make Run_MPI_ParallelAdd
```
