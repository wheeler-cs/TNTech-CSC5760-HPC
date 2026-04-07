/**
 * Given an unsorted array of N integers on process 0, partition the data into 3
 * buckets using 2 pivots, distribute each bucket to one MPI process, sort
 * locally with quicksort, and gather the results back to process 0 in sorted 
 * order.
 * 
 * For some information on quick sort, check out
 * https://www.geeksforgeeks.org/dsa/quick-sort-algorithm/.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <mpi.h>

//#define DEBUG

#ifdef DEBUG
#define DBGPRINT(...) \
        printf("[DEBUG] "); \
        printf(__VA_ARGS__); \
        fflush(stdout);
#else
// Disable debug printing if undefined
#define DBGPRINT(...)
#endif

#define VECTOR_LEN   10
#define LOWER_BOUND   0
#define UPPER_BOUND 100
#define ROOT_NODE     0

enum MessageTags
{
    TAG_VECTOR_LEN,
    TAG_VECTOR_DATA,
};


void populateVector(int vector[VECTOR_LEN])
{
    int i;
    
    srand(time(NULL));
    for(i = 0; i < VECTOR_LEN; i++)
    {
        vector[i] = (rand() % (UPPER_BOUND - LOWER_BOUND)) + LOWER_BOUND;
    }
}

void printVector(int vector[VECTOR_LEN])
{
    int i;

    printf("[ ");
    for(i = 0; i < VECTOR_LEN; i++)
    {
        printf("%2d ", vector[i]);
    }
    printf("]\n");
}

void calculatePivots(int * pivotLow, int * pivotHigh)
{
    *pivotLow  = (UPPER_BOUND / 3) + LOWER_BOUND;
    *pivotHigh = ((UPPER_BOUND / 3) * 2) + LOWER_BOUND;

    DBGPRINT("L: %d, U: %d\n", *pivotLow, *pivotHigh)
}

// So many function parameters it looks like a Windows API call
int * createBucket(int lowerBound, int upperBound, int inputVector[VECTOR_LEN], int * bucketSize)
{
    int i,
        * bucket;
    bucket = NULL;
    *bucketSize = 0;

    // Sub-divide vector into buckets
    for(i = 0; i < VECTOR_LEN; i++)
    {
        if((inputVector[i] >= lowerBound) && (inputVector[i] < upperBound))
        {
            *bucketSize += 1;
            bucket = realloc(bucket, (*bucketSize) * sizeof(int));
            bucket[*bucketSize - 1] = inputVector[i];
        }
    }

    return bucket;
}

void freeBucket(int * bucket)
{
    // Only free when not already null
    if(bucket != NULL)
    {
        free(bucket);
        bucket = NULL; // Dangling pointer
    }
}

void printBucket(int * bucket, int bucketSize)
{
    int i;

    printf("[ ");
    for(i = 0; i < bucketSize; i++)
    {
        printf("%2d ", *(bucket + i));
    }
    printf("]\n");
}

void sendBucket(int dest, int * bucket, int bucketSize)
{
    DBGPRINT("Sending data to %d\n", dest)
    // Perform blocking sends because we must know how much data we need to receive first
    MPI_Send(&bucketSize, 1,          MPI_INT, dest, TAG_VECTOR_LEN,  MPI_COMM_WORLD);
    MPI_Send(bucket,      bucketSize, MPI_INT, dest, TAG_VECTOR_DATA, MPI_COMM_WORLD);
}

void recvBucket(int src, int ** bucket, int * bucketSize)
{
    // Perform blocking receives because we must know how much data we need to receive first
    MPI_Recv(bucketSize, 1, MPI_INT, src, TAG_VECTOR_LEN,  MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *bucket = (int *)malloc((*bucketSize) * sizeof(int));
    MPI_Recv(*bucket, *bucketSize, MPI_INT, src, TAG_VECTOR_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    DBGPRINT("Received bucket data from root\n")
}

// Quick sort adapted from https://www.geeksforgeeks.org/dsa/quick-sort-algorithm/

int qsPartition(int ** bucket, int low, int high)
{
    int pivot, i, j, temp;

    pivot = (*bucket)[high];
    i = low - 1;

    for(j = low; j <= high - 1; j++)
    {
        if((*bucket)[j] < pivot)
        {
            i++;
            temp = (*bucket)[i];
            (*bucket)[i] = (*bucket)[j];
            (*bucket)[j] = temp;
        }
    }

    temp = (*bucket)[i + 1];
    (*bucket)[i + 1] = (*bucket)[high];
    (*bucket)[high] = temp;
    return (i + 1);
}

void qsBucket(int ** bucket, int low, int high)
{
    int pivot;
    if(low < high)
    {
        pivot = qsPartition(bucket, low, high);
        qsBucket(bucket, low,       pivot - 1);
        qsBucket(bucket, pivot + 1, high);
    }
}


int main(int argc, char ** argv)
{
    // Operation variables
    int vector[VECTOR_LEN];
    int i, rank, size,
        pivotLow, pivotHigh,
        * bucketA, * bucketB, * bucketC,
        bucketASize, bucketBSize, bucketCSize;
    bucketA = NULL;
    bucketB = NULL;
    bucketC = NULL;

    // MPI setup
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Do additional setup if root node
    if(rank == ROOT_NODE)
    {
        // Setup and print initial vector to sort
        printf("Input Vector:  ");
        populateVector(vector);
        printVector(vector);

        // Abusing the fact that we know before hand that we only need two pivots
        calculatePivots(&pivotLow, &pivotHigh);

        // Divide vector into buckets
        bucketA = createBucket(LOWER_BOUND, pivotLow,    vector, &bucketASize);
        bucketB = createBucket(pivotLow,    pivotHigh,   vector, &bucketBSize);
        bucketC = createBucket(pivotHigh,   UPPER_BOUND, vector, &bucketCSize);

    #ifdef DEBUG
        printBucket(bucketA, bucketASize);
        printBucket(bucketB, bucketBSize);
        printBucket(bucketC, bucketCSize);
    #endif

        // Send buckets to other procs
        sendBucket(1, bucketB, bucketBSize);
        sendBucket(2, bucketC, bucketCSize);
    }
    // Get bucket from root node
    else
    {
        recvBucket(ROOT_NODE, &bucketA, &bucketASize);
    }
    
    qsBucket(&bucketA, 0, bucketASize - 1);

    // Send back sorted data
    if(rank != ROOT_NODE)
    {
        sendBucket(ROOT_NODE, bucketA, bucketASize);
    }
    // Recombine sorted data
    else
    {
        recvBucket(1, &bucketB, &bucketBSize);
        recvBucket(2, &bucketC, &bucketCSize);

        for(i = 0; i < bucketASize; i++)
        {
            vector[i] = bucketA[i];
        }
        for(i = 0; i < bucketBSize; i++)
        {
            vector[i + bucketASize] = bucketB[i];
        }
        for(i = 0; i < bucketCSize; i++)
        {
            vector[i + bucketASize + bucketBSize] = bucketC[i];
        }

        printf("Output Vector: ");
        printVector(vector);
    }

    // Clean up
    free(bucketA);
    free(bucketB);
    free(bucketC);
    MPI_Finalize();

    return 0;
}
