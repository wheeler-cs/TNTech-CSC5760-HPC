#include "problem3.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char ** argv)
{
    // Allocate memory for vectors
    srand(time(NULL));
    int * v1, * v2;
    v1 = populateVector();
    v2 = populateVector();
    //printVector(v1);
    //printVector(v2);

    // Initialize program
    int rank, size;
    double startTime, endTime;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // Start timer for calculation time
    if(!rank)
    {
        startTime = MPI_Wtime();
    }

    // Calculate product of subvectors
    int product, start, end;
    start = (rank * VECTOR_LEN) / size;
    end = ((rank + 1) * VECTOR_LEN) / size;
    product = vectorVectorMult(v1, v2, start, end);

    // Collect calculated vector products using tree
    int multiplier, value;
    for (multiplier = 1; multiplier < size; multiplier *= 2)
    {
        // Rank is a receiver
        if (rank % (2 * multiplier) == 0)
        {
            // Do not receive from outside range
            if (rank + multiplier < size)
            {
                MPI_Recv(&value, 1, MPI_INT, rank + multiplier, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                product += value;
            }
        }
        // Rank is sender
        else
        {
            int dest = rank - multiplier;
            MPI_Send(&product, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            break; // Sender doesn't do anymore work after sending
        }
    }

    // Print vector product if root in tree
    if(!rank)
    {
        // Stop timer for calculation time
        endTime = MPI_Wtime();
        printf("\nVector Product: %d", product);
        printf("\nCalculation took %lf seconds\n", endTime-startTime);
    }

    // Cleanup
    MPI_Finalize();
    free(v1);
    v1 = NULL;
    free(v2);
    v2 = NULL;
    fflush(stdout);

    return 0;
}
