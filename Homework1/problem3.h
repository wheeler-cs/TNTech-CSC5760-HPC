#ifndef _PROBLEM3_H
#define _PROBLEM3_H

#include <mpi.h>

#define VECTOR_LEN 1024
#define ELEMENT_LIM 10


int * populateVector();
void printVector(int *);

int vectorVectorMult(int *, int *, int, int);

#endif