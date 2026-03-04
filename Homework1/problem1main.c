#include "problem1.h"

#include <stdio.h>


int main(int argc, char ** argv)
{
    // Declare local variables
    int rank, size, result;
    rank = 0;
    size = 0;
    result = 0;

    // Init MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Perform operation and print result
    result = ringPass(&rank, &size);
    if(!rank) // Only rank 0 should print result
    {
        printf("\nLaps Around Ring: %d", result);
    }

    // Cleanup
    printf("\n");
    fflush(stdout);
    MPI_Finalize();
    return 0;
}
