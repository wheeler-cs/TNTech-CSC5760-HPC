#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mpi.h>

//#define DEBUG_PRINT

#define PROC_ROOT 0
#define VECTOR_LEN 10

enum CollectTags
{
    TAG_POS_START,
    TAG_POS_END,
    TAG_VECTOR,
};


void initVector(int vector[VECTOR_LEN])
{
    unsigned int i;
    for(i = 0; i < VECTOR_LEN; i++)
    {
        vector[i] = (rand() % 10) + 1;
    }
}

void blankInitVector(int vector[VECTOR_LEN])
{
    unsigned int i;
    for(i = 0; i < VECTOR_LEN; i++)
    {
        vector[i] = 0;
    }
}


void calculateRange(int rank, int size, int * start, int * end)
{
    int stride,
        procsWithExtra;

    procsWithExtra = VECTOR_LEN % size;
    stride = VECTOR_LEN / size;

    // Determine the number of elements to add
    if(rank < procsWithExtra)
    {
        stride++;
    }

    // Calculate starting position
    if(rank < procsWithExtra)
    {
        *start = (rank * stride);
    }
    else
    {
        *start = (procsWithExtra * (stride + 1));
        *start += ((rank - procsWithExtra) * stride);
    }

    // Calculate ending position
    *end = *start + stride;

#ifdef DEBUG_PRINT
    printf("[DEBUG] Rank: %d [%d %d)\n", rank, *start, *end);
#endif

}

void addVectors(int vA[VECTOR_LEN], int vB[VECTOR_LEN], int vC[VECTOR_LEN], int start, int end)
{
    int i;
    for(i = start; i < end; i++)
    {
        vC[i] = vA[i] + vB[i];
    }
}

void printVector(int vector[VECTOR_LEN])
{
    unsigned int i;
    for(i = 0; i < VECTOR_LEN; i++)
    {
        printf("%2d ", vector[i]);
    }
    printf("\n");
}

void collectSend(int rank, int size, int start, int end, int vector[VECTOR_LEN])
{
    MPI_Request reqs[3];
    if(rank == 0)
    {
        int i, j, tempStart, tempEnd,
            tempVec[VECTOR_LEN];
        for(i = 1; i < size; i++)
        {
            // Get start, end, vector data from other process
            MPI_Irecv(&tempStart,
                     1,
                     MPI_INT,
                     i,
                     TAG_POS_START,
                     MPI_COMM_WORLD,
                     &reqs[0]);
            MPI_Irecv(&tempEnd,
                     1,
                     MPI_INT,
                     i,
                     TAG_POS_END,
                     MPI_COMM_WORLD,
                     &reqs[1]);
            MPI_Irecv(&tempVec,
                      VECTOR_LEN,
                      MPI_INT,
                      i,
                      TAG_VECTOR,
                      MPI_COMM_WORLD,
                      &reqs[2]);
            MPI_Waitall(3, reqs, MPI_STATUSES_IGNORE);
        #ifdef DEBUG_PRINT
            printf("[DEBUG] Received from %d: [%d, %d)\n", i, tempStart, tempEnd);
        #endif
            for(j = tempStart; j < tempEnd; j++)
            {
                vector[j] = tempVec[j];
            }
        }
    }
    else
    {
        // Send start, end, and vector data to root aggregator
        MPI_Isend(&start,
                  1,
                  MPI_INT,
                  PROC_ROOT,
                  TAG_POS_START,
                  MPI_COMM_WORLD,
                  &reqs[0]);
        MPI_Isend(&end,
                  1,
                  MPI_INT,
                  PROC_ROOT,
                  TAG_POS_END,
                  MPI_COMM_WORLD,
                  &reqs[1]);
        MPI_Isend(vector,
                  VECTOR_LEN,
                  MPI_INT,
                  PROC_ROOT,
                  TAG_VECTOR,
                  MPI_COMM_WORLD,
                  &reqs[2]);
        MPI_Waitall(3, reqs, MPI_STATUSES_IGNORE);
    }
}


int main(int argc, char ** argv)
{
    // Variables for operation
    int rank, size,
        start, end,
        vectorA[VECTOR_LEN],
        vectorB[VECTOR_LEN],
        vectorC[VECTOR_LEN];
    MPI_Request reqs[2];


    // MPI init
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Init addend vectors with random values
    if(rank == 0)
    {
        srand(time(NULL));
        initVector(vectorA);
        initVector(vectorB);
        blankInitVector(vectorC);
    }
    /**
     * NOTE: A more efficient implementation would probably use dynamically-
     * allocated memory to save on space complexity. I did not do that because I
     * am lazy and wanted to practice using MPI more than implementing a highly-
     * optimized application.
     */
    // Distribute vectors to other processes
    MPI_Ibcast(vectorA, VECTOR_LEN, MPI_INT, PROC_ROOT, MPI_COMM_WORLD, &reqs[0]);
    MPI_Ibcast(vectorB, VECTOR_LEN, MPI_INT, PROC_ROOT, MPI_COMM_WORLD, &reqs[1]);
    MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

    // Figure which vals to add and add them
    calculateRange(rank, size, &start, &end);
    addVectors(vectorA, vectorB, vectorC, start, end);
    collectSend(rank, size, start, end, vectorC);

    // Print vectors when done
    if(rank == 0)
    {
        printVector(vectorA);
        printVector(vectorB);
        printVector(vectorC);
    }

    // Clean up
    MPI_Finalize();

    return 0;
}
