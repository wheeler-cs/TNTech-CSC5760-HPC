/** CODE GENERATED USING CHATGPT 5 FREE
 * 
 * PROMPT: Write an MPI program that passes a message of one integer around in a logical ring of
 * processes with MPI COMM WORLD. The integer should start at 0 in process 0 and be incremented each
 * time it passes around the ring, and you should be able to have the message go around the ring N
 * times, where N is specified at compile time.
 * 
 * LLM structued code with if-else on out side and for loop on inside, while code I wrote has the
 * for loop on the outside and the if-else statements on the inside. LLM code is also monolithic
 * where mine is built using functions.
 */


#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef N
#define N 3   // Default number of times the message goes around the ring
#endif

int main(int argc, char *argv[]) {
    int rank, size;
    int value = 0;
    int next, prev;
    int tag = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    next = (rank + 1) % size;
    prev = (rank - 1 + size) % size;

    if (rank == 0) {
        value = 0;
        printf("Process %d starting ring with value %d\n", rank, value);

        // Start the ring
        MPI_Send(&value, 1, MPI_INT, next, tag, MPI_COMM_WORLD);

        for (int i = 0; i < N; i++) {
            // Receive from last process
            MPI_Recv(&value, 1, MPI_INT, prev, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            value++;  // Increment after full circulation
            printf("Process %d completed round %d, value now %d\n", rank, i+1, value);

            if (i < N - 1) {
                MPI_Send(&value, 1, MPI_INT, next, tag, MPI_COMM_WORLD);
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            MPI_Recv(&value, 1, MPI_INT, prev, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&value, 1, MPI_INT, next, tag, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}

