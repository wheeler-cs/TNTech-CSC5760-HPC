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
#include <time.h>

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
    TAG_VECTOR_DATA,
};

struct Subvector
{
    int vectorLen,
      * vector;
};

int randPopVector(int * vector, int vectorLen)
{
    int i;

    srand(time(NULL));

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

void printSubvector(struct Subvector * subvector)
{
    int i;
    printf("[ ");
    for(i = 0; i < subvector->vectorLen; i++)
    {
        printf("%d ", subvector->vector[i]);
    }
    printf("]\n");
}

int calcStride(int vectorLen, int divisions, int rank)
{
    int stride,
        extra;
    
    stride = vectorLen / divisions;
    extra  = vectorLen % divisions;

    if(rank < extra)
    {
        stride++;
    }

    return stride;
}

void deallocSubvector(struct Subvector * subvectors, int vectorCount)
{
    int i;

    if(subvectors != NULL)
    {
        for(i = 0; i < vectorCount; i++)
        {
            free(subvectors[i].vector);
        }
        free(subvectors);
    }
}

struct Subvector * linearDistribute(int * vector, int vectorLen, MPI_Comm comm, int * vectorCount)
{
    // Function variables
    int stride, i, iVector, j;
    struct Subvector * subvectors;

    // Prepare function variables
    MPI_Comm_size(comm, vectorCount);
    subvectors = calloc(*vectorCount, sizeof(struct Subvector));

    for(i = 0, iVector = 0; i < *vectorCount; i++)
    {
        subvectors[i].vectorLen = calcStride(M, P, i);
        subvectors[i].vector = calloc(subvectors[i].vectorLen, sizeof(int));
        for(j = 0; j < subvectors[i].vectorLen; j++, iVector++)
        {
            subvectors[i].vector[j] = vector[iVector];
        }
    }

    return subvectors;
}

void sendSubvectors(struct Subvector * subvectors, int vectorCount, MPI_Comm comm)
{
    int i;
    // Assume root sender is rank 0
    for(i = 1; i < vectorCount; i++)
    {
        // Use blocking sends because I'm lazy
        MPI_Send(&subvectors[i].vectorLen, 1, MPI_INT, i, TAG_VECTOR_LEN, comm);
        MPI_Send(subvectors[i].vector, subvectors[i].vectorLen, MPI_INT, i, TAG_VECTOR_DATA, comm);
    }
}

struct Subvector recvSubvector(int root, MPI_Comm comm)
{
    struct Subvector subvector;

    MPI_Recv(&subvector.vectorLen, 1, MPI_INT, 0, TAG_VECTOR_LEN, comm, MPI_STATUS_IGNORE);
    subvector.vector = calloc(subvector.vectorLen, sizeof(int));
    MPI_Recv(subvector.vector, subvector.vectorLen, MPI_INT, 0, TAG_VECTOR_DATA, comm, MPI_STATUS_IGNORE);

    return subvector;
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
    int rowIdx, colIdx,
        rowCount, colCount;
    MPI_Comm columns, rows;
    MPI_Comm_split(MPI_COMM_WORLD, rank / Q, rank, &columns);
    MPI_Comm_rank(columns, &colIdx);
    MPI_Comm_split(MPI_COMM_WORLD, rank % Q, rank, &rows);
    MPI_Comm_rank(rows, &rowIdx);

    // Distribute subvectors to other processes
    int vectorCount, i;
    struct Subvector * subvectors,
                       subvector;
    if(rank == ROOT_NODE)
    {
        subvectors = linearDistribute(vector, M, rows, &vectorCount);
        sendSubvectors(subvectors, vectorCount, rows);
        // Copy vector with index 0 to root node
        subvector.vectorLen = subvectors[0].vectorLen;
        subvector.vector = calloc(subvector.vectorLen, sizeof(int));
        for(i = 0; i < subvector.vectorLen; i++)
        {
            subvector.vector[i] = subvectors[0].vector[i];
        }
    }
    else if(colIdx == 0)
    {
        subvector = recvSubvector(ROOT_NODE, rows);
    }
    
    // Clean up
    deallocSubvector(subvectors, vectorCount);
    free(subvector.vector);
    fflush(stdout);
    MPI_Finalize();
    return 0;
}
