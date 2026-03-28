/**
 * Given an unsorted array of N integers, sort it in ascending order using a
 * parallel merge sort algorithm with MPI, where the array is evenly distributed
 * across P processes, each process sorts its local chunk, and results are
 * merged back to process 0 through a binary tree reduction.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mpi.h>

// === Defines ===

//#define DEBUG_PRINT     // Enable terminal output for debugging

#define ROOT_NODE  0
#define VECTOR_LEN 11

enum MessageTags
{
    TAG_VECTOR_START,
    TAG_VECTOR_END,
    TAG_VECTOR_DATA,
};

// === Helper Functions ===

void initVector(int vector[VECTOR_LEN])
{
    unsigned int i;
    for(i = 0; i < VECTOR_LEN; i++)
    {
        vector[i] = (rand() % 10) + 1;
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
}

// NOTE: Algorithm for merge sort sourced from https://www.geeksforgeeks.org/dsa/merge-sort/
// (I haven't written a merge sort in literally 5 years)
void merge(int arr[VECTOR_LEN], int l, int m, int r){
    
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    int L[n1], R[n2];

    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(int vector[VECTOR_LEN], int left, int right)
{
    int average;
    if(left < right)
    {
        average = left + (right - left) / 2;

        mergeSort(vector, left, average);
        mergeSort(vector, average + 1, right);

        merge(vector, left, average, right);
    }
}

void printVector(int vector[VECTOR_LEN])
{
    unsigned int i;
    for(i = 0; i < VECTOR_LEN; i++)
    {
        printf("%d ", vector[i]);
    }
    printf("\n");
}

void sendVector(int dest, int start, int end, int vector[VECTOR_LEN])
{
    MPI_Request reqs[3];

    // Send vector and bounds
    MPI_Isend(&start, 1,          MPI_INT, dest, TAG_VECTOR_START, MPI_COMM_WORLD, &reqs[0]);
    MPI_Isend(&end,   1,          MPI_INT, dest, TAG_VECTOR_END,   MPI_COMM_WORLD, &reqs[1]);
    MPI_Isend(vector, VECTOR_LEN, MPI_INT, dest, TAG_VECTOR_DATA,  MPI_COMM_WORLD, &reqs[2]);
    MPI_Waitall(3, reqs, MPI_STATUSES_IGNORE);
}

// NOTE: Merge sort is handled after every iteration of the tree reduction to sort the subvectors
void recvVector(int src, int * start, int * end, int vector[VECTOR_LEN])
{
    int i,
        tempStart, tempEnd,
        tempVector[VECTOR_LEN];
    MPI_Request reqs[3];

    // Receive vector and bounds
    MPI_Irecv(&tempStart, 1,          MPI_INT, src, TAG_VECTOR_START, MPI_COMM_WORLD, &reqs[0]);
    MPI_Irecv(&tempEnd,   1,          MPI_INT, src, TAG_VECTOR_END,   MPI_COMM_WORLD, &reqs[1]);
    MPI_Irecv(tempVector, VECTOR_LEN, MPI_INT, src, TAG_VECTOR_DATA,  MPI_COMM_WORLD, &reqs[2]);
    MPI_Waitall(3, reqs, MPI_STATUSES_IGNORE);
    
    // Copy data from received vector
    for(i = tempStart; i < tempEnd; i++)
    {
        vector[i] = tempVector[i];
    }

#ifdef DEBUG_PRINT
    printf("[DEBUG] Received: ");
    printVector(vector);
#endif

    // Set new bounds
    if(tempStart < *start)
    {
        *start = tempStart;
    }
    if(tempEnd > *end)
    {
        *end = tempEnd;
    }

    // Sort subvector
    mergeSort(vector, *start, *end - 1);
}

void treeCollect(int rank, int size, int * start, int * end, int vector[VECTOR_LEN])
{
    int multiplier, src, dest;
    // Perform tree collection loop
    for (multiplier = 1; multiplier < size; multiplier *= 2)
    {
        // Rank is a receiver
        if (rank % (2 * multiplier) == 0)
        {
            // Do not receive from outside range
            if (rank + multiplier < size)
            {
                src = rank + multiplier;
                recvVector(src, start, end, vector);
            }
        }
        // Rank is sender
        else
        {
            dest = rank - multiplier;
            sendVector(dest, *start, *end, vector);
            break; // Sender doesn't do anymore work after sending
        }
    }
}

// === Main ===

int main(int argc, char ** argv)
{
    // Program vars
    int rank, size,
        start, end,
        vector[VECTOR_LEN];

    // Init program
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    calculateRange(rank, size, &start, &end);
    if(rank == ROOT_NODE)
    {
        // Put random variables in vector to sort
        srand(time(NULL));
        initVector(vector);
        // Print starting vector to terminal
        printf("Start: ");
        printVector(vector);
    }

    // Distribute vector to processes and do initial sort
    MPI_Bcast(vector, VECTOR_LEN, MPI_INT, ROOT_NODE, MPI_COMM_WORLD);
    mergeSort(vector, start, end - 1);

    treeCollect(rank, size, &start, &end, vector);

    // Print sorted vector to terminal
    if(rank == ROOT_NODE)
    {
        printf("End: ");
        printVector(vector);
    }

    // Clean up
    MPI_Finalize();

    return 0;
}
