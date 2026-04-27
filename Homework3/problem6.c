/**
 * Modify Problem #2 above, but store y in a scatter distribution
 * (aka wrap-mapped) distribution. For this case, from global coeffient J on Q
 * processes, then local index is j = J/Q, q = J mod Q, and the number of 
 * elements per process is the same as in the linear load-balanced distribution
 * would produce with N elements over Q partitions. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <mpi.h>

//#define DEBUG

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

struct Subvector * linearDistribute(int * vector, int vectorLen, int divisor, MPI_Comm comm, int * vectorCount)
{
    // Function variables
    int stride, i, iVector, j;
    struct Subvector * subvectors;

    // Prepare function variables
    MPI_Comm_size(comm, vectorCount);
    DBGPRINT("Communicator Size: %d", *vectorCount)
    subvectors = calloc(*vectorCount, sizeof(struct Subvector));

    for(i = 0, iVector = 0; i < *vectorCount; i++)
    {
        subvectors[i].vectorLen = calcStride(M, divisor, i);
        subvectors[i].vector = calloc(subvectors[i].vectorLen, sizeof(int));
        for(j = 0; j < subvectors[i].vectorLen; j++, iVector++)
        {
            subvectors[i].vector[j] = vector[iVector];
        }
    }

    return subvectors;
}

struct Subvector * scatterDistribute(int * vector, int vectorLen, int divisor, MPI_Comm comm, int * vectorCount)
{
    int i;
    struct Subvector * subvectors;

    // Determine number of subvectors
    MPI_Comm_size(comm, vectorCount);
    subvectors = calloc(*vectorCount, sizeof(struct Subvector));

    // Allocate memory for subvectors
    for(i = 0; i < *vectorCount; i++)
    {
        subvectors[i].vectorLen = calcStride(M, divisor, i);
        subvectors[i].vector = calloc(subvectors[i].vectorLen, sizeof(int));
    }

    // Distribute elements to subvector
    for(i = 0; i < vectorLen; i++)
    {
        subvectors[i % divisor].vector[i / divisor] = vector[i];
    }

    return subvectors;
}

void sendSubvectors(struct Subvector * subvectors, int vectorCount, MPI_Comm comm)
{
    int i;
    // Assume root sender is rank 0
    for(i = 1; i < vectorCount; i++)
    {
        // Use blocking sends because I'm lazy, Scatterv could work here too
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

int calcDotProduct(int * vectorA, int vectorALen, int * vectorB, int vectorBLen)
{
    int i, j, dotProduct;
    for(i = 0; i < vectorALen; i++)
    {
        for(j = 0; j < vectorBLen; j++)
        {
            dotProduct += vectorA[i] * vectorB[j];
        }
    }

    return dotProduct;
}


int main(int argc, char ** argv)
{
    // Program initialization
    int size, rank,
        vector[M];
    srand(time(NULL));
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if(rank == ROOT_NODE)
    {
        randPopVector(vector, M);
        printf("Input horizontal vector: ");
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

    // Linearly distribute vector in column
    int vectorCount, i;
    struct Subvector * subvectors,
                       subvector;
    if(rank == ROOT_NODE)
    {
        subvectors = linearDistribute(vector, M, P, rows, &vectorCount);
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

    // Copy vector across row
    MPI_Bcast(&subvector.vectorLen, 1, MPI_INT, 0, columns);
    if(colIdx != 0)
    {
        subvector.vector = calloc(subvector.vectorLen, sizeof(int));
    }
    MPI_Bcast(subvector.vector, subvector.vectorLen, MPI_INT, 0, columns);

    // Wait for all operations to complete before moving on
    fflush(stdout);
    sleep(1);

    // Prepare variables for vertical distribution
    int yVectorCount,
        yVector[M];
    struct Subvector * ySubvectors,
                       ySubvector;

    // Distribute vector vertically
    if(rank == ROOT_NODE)
    {
        randPopVector(yVector, M);
        printf("Input vertical vector: ");
        printVector(yVector, M);
        ySubvectors = scatterDistribute(yVector, M, Q, columns, &yVectorCount);
        sendSubvectors(ySubvectors, yVectorCount, columns);
        // Copy vector with index 0 to root node
        ySubvector.vectorLen = ySubvectors[0].vectorLen;
        ySubvector.vector = calloc(ySubvector.vectorLen, sizeof(int));
        for(i = 0; i < ySubvector.vectorLen; i++)
        {
            ySubvector.vector[i] = ySubvectors[0].vector[i];
        }
    }
    else if(rowIdx == 0)
    {
        ySubvector = recvSubvector(ROOT_NODE, columns);
    }

    // Copy vector across row
    MPI_Bcast(&ySubvector.vectorLen, 1, MPI_INT, 0, rows);
    if(rowIdx != 0)
    {
        ySubvector.vector = calloc(ySubvector.vectorLen, sizeof(int));
    }
    MPI_Bcast(ySubvector.vector, ySubvector.vectorLen, MPI_INT, 0, rows);

    // Compute dot product
    int dotProduct;
    dotProduct = calcDotProduct(subvector.vector,
                                subvector.vectorLen,
                                ySubvector.vector,
                                ySubvector.vectorLen);

    // Reassimilate sums across row
    int fullDotProduct;
    MPI_Allreduce(&dotProduct,
                  &fullDotProduct,
                  1,
                  MPI_INT,
                  MPI_SUM,
                  columns);

    // Reassimilate sums across column
    MPI_Allreduce(&fullDotProduct,
                  &dotProduct,
                  1,
                  MPI_INT,
                  MPI_SUM,
                  rows);

    printf("Final Sum: %d\n", dotProduct);
    
    // Clean up
    deallocSubvector(subvectors, vectorCount);
    free(subvector.vector);
    deallocSubvector(ySubvectors, yVectorCount);
    free(ySubvector.vector);
    fflush(stdout);
    MPI_Finalize();
    return 0;
}
