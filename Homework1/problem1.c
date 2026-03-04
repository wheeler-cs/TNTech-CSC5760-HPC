#include "problem1.h"

#include <stdio.h>

int ringPass(int * rank, int * size)
{
    int i, localVal;
    localVal = 0;
    for(i = 0; i < NUM_ROUNDS; i++)
    {
        // Rank zero, increment value and start new lap 
        if(!(*rank))
        {
            if(i)
            {
                MPI_Recv(&localVal, 1, MPI_INT, *rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            localVal += 1;
            MPI_Send(&localVal, 1, MPI_INT, (*rank + 1) % *size, 0, MPI_COMM_WORLD);
        }
        // Rank > 0, just forward message to next rank
        else
        {
            MPI_Recv(&localVal, 1, MPI_INT, *rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&localVal, 1, MPI_INT, (*rank + 1) % *size, 0, MPI_COMM_WORLD);
        }
    }

    return localVal;
}
