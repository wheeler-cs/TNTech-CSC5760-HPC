/**
 * Write a program that uses MPI Comm split to create two distinct sets of
 * sub-communicators for all processes in MPI COMM WORLD. The world size must be
 * exactly P × Q for P, Q ≥ 1. In the first split, group processes that have the
 * same color when their ranks are divided by the integer Q; within these
 * groups, perform an MPI_Reduce to calculate the sum of the world ranks and
 * have the root of each sub-communicator print the result. In the second split,
 * group processes together that have the same color when the color is computed
 * as rank modulo Q; within these groups, have the root process perform an
 * MPI_Bcast of its original world rank to all other processes in that
 * sub-communicator. Each process should then print the value it received to
 * demonstrate that the collective communication was isolated to processes of
 * the same color.
 */

#include <stdio.h>

#include <mpi.h>

#define ROOT_NODE 0

#define P 3
#define Q 2

int main(int argc, char ** argv)
{
    int rank, size;

    // Initalize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Create comm based on div Q
    MPI_Comm divQ;
    int divQRank, rankSum;
    MPI_Comm_split(MPI_COMM_WORLD, rank / Q, rank, &divQ);
    MPI_Comm_rank(divQ, &divQRank);
    // Perform summation reduction
    MPI_Reduce(&rank, &rankSum, 1, MPI_INT, MPI_SUM, 0, divQ);
    if(divQRank == ROOT_NODE)
    {
        printf("Sum for group %d: %d\n", rank / Q, rankSum);
    }

    // Creat comm based on mod Q
    MPI_Comm modQ;
    int modQRank;
    MPI_Comm_split(MPI_COMM_WORLD, rank % Q, rank, &modQ);
    MPI_Comm_rank(modQ, &modQRank);
    // Broadcast original rank
    if(modQRank == ROOT_NODE)
    {
        modQRank = rank;
    }
    MPI_Bcast(&modQRank, 1, MPI_INT, ROOT_NODE, modQ);
    printf("Rank: %d\n", modQRank);

    // Clean up
    fflush(stdout);
    MPI_Finalize();

    return 0;
}
