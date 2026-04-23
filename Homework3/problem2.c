/**
 * Write an MPI program that builds a 2D process topology of shape PxQ. On each
 * column of processes, store a vector x of length M, distributed in a linear
 * load-balanced fashion “vertically” (it will be replicated Q times). Start
 * with data only in process (0,0), and scatter it down the first column. Once
 * it is scattered on column 0, broadcast it horizontally in each process row.
 * Allocate a vector y of length M that is replicated “horizontally” in each
 * process row and stored also in linear load-balanced distribution; there will
 * be P replicas, one in each process row. Using MPI Allreduce or MPI Allgather
 * with the appropriate communicators, do the parallel copy y := x. There should
 * be P replicas of the answer in y when you’re done.
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#define DEBUG

#ifdef DEBUG
#define DBGPRINT(...) \
        printf("[DEBUG] "); \
        printf(__VA_ARGS__); \
        printf("\n"); \
        fflush(stdout);
#else
// Disable debug printing if undefined
#define DBGPRINT(...)
#endif

#define ROOT_NODE  0
#define UPPER_LIM 10

#define P  4
#define Q  2
#define M 15

enum MessageTags
{
    TAG_VECTOR_LEN,
    TAV_VECTOR_DATA,
};

int randPopVector(int * vector, int vectorLen)
{
    int i;
    for(i = 0; i < vectorLen; i++)
    {
        vector[i] = (rand() % UPPER_LIM) + 1;
    }
}

void printVector(int * vector, int vectorLen)
{
    int i;

    if(vector != NULL)
    {
        printf("[ ");
        for(i = 0; i < vectorLen; i++)
        {
            printf("%d ", vector[i]);
        }
        printf("]\n");
    }
}

int calculateStride(int rank, int m, int p)
{
    int stride,
        extra;

    stride = m / p;
    extra  = m % p;

    if(rank < extra)
    {
        stride++;
    }

    return stride;
}

int calculateOffset(int rank, int * strides)
{
    int i, offset;
    offset = 0;

    for(i = 0; i < rank; i++)
    {
        offset += strides[i];
    }

    return offset;
}



int main(int argc, char ** argv)
{
    // Program initialization
    int size, rank,
        vector[M],
        startIdx[P];
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if(rank == ROOT_NODE)
    {
        randPopVector(vector, M);
        printf("Input vector: ");
        printVector(vector, M);
    }

    // Create row and column groups
    int rowRank, colRank,
        rowCount, colCount;
    MPI_Comm columns, rows;
    MPI_Comm_split(MPI_COMM_WORLD, rank / Q, rank, &columns);
    MPI_Comm_rank(columns, &colRank);
    MPI_Comm_size(columns, &colCount);
    MPI_Comm_split(MPI_COMM_WORLD, rank / P, rank, &rows);
    MPI_Comm_rank(rows, &rowRank);
    MPI_Comm_size(rows, &rowCount);
    if(rank == ROOT_NODE)
    {
        DBGPRINT("Row Count (%d) || Col Count (%d)", rowCount, colCount)
    }

    // Distribute vector to row groups
    int i,
        * strides,
        * offsets,
        * subvector;
    strides = NULL;
    offsets = NULL;
    subvector = NULL;
    // Allocate memory for strides and offsets
    strides = calloc(rowCount, sizeof(int));
    offsets = calloc(rowCount, sizeof(int));
    // Calculate how to distribute vector
    for(i = 0; i < rowCount; i++)
    {
        strides[i] = calculateStride(i, M, rowCount);
    }
    for(i = 0; i < rowCount; i++)
    {
        offsets[i] = calculateOffset(i, strides);
    }
    // Distribute vector as subvector
    subvector = calloc(strides[rowRank], sizeof(int));
    if(colRank != ROOT_NODE)
    {
        printVector(subvector, strides[rowRank]);
    }
    MPI_Scatterv(vector,
                 strides,
                 offsets,
                 MPI_INT,
                 subvector,
                 strides[rowRank],
                 MPI_INT,
                 ROOT_NODE,
                 rows);

    // Broadcast vector to row group
    MPI_Bcast(subvector, strides[rowRank], MPI_INT, ROOT_NODE, columns);
    //printVector(subvector, strides[rowRank]);

    // Clean up
    free(strides);
    free(offsets);
    free(subvector);
    fflush(stdout);
    MPI_Finalize();
    return 0;
}
