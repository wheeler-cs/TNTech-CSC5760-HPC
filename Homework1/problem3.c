#include "problem3.h"

#include <stdio.h>
#include <stdlib.h>

int * populateVector()
{
    int i;
    int * vector = malloc(VECTOR_LEN * sizeof(int));
    for(i = 0; i < VECTOR_LEN; i++)
    {
        vector[i] = rand() % ELEMENT_LIM;
    }
    return vector;
}

void printVector(int * vector)
{
    int i;
    printf("\n[ ");
    for(i = 0; i < VECTOR_LEN; i++)
    {
        printf("%d ", vector[i]);
    }
    printf("]");
}

int vectorVectorMult(int * vectorA, int * vectorB, int start, int end)
{
    int i, product;
    product = 0;
    for(i = start; i < end; i++)
    {
        product += vectorA[i] * vectorB[i];
    }
    return product;
}
