#include <mpi.h>
#include <stdio.h>

#define Q 3

int main(int argc, char ** argv)
{
    int rank, size,
        divRank, divSize,
        modRank, modSize;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Split communicators
    MPI_Comm divQComm, modQComm;
    MPI_Comm_split(MPI_COMM_WORLD, (int)(rank / Q), rank, &divQComm);
    MPI_Comm_split(MPI_COMM_WORLD, rank % Q, rank, &modQComm);

    // Get new ranks and sizes for subcommunicators
    MPI_Comm_rank(divQComm, &divRank);
    MPI_Comm_size(divQComm, &divSize);
    MPI_Comm_rank(modQComm, &modRank);
    MPI_Comm_size(modQComm, &modSize);

    // Print out rank and size information
    printf("\n[MPI_COMM_WORLD] Rank: %d, Size: %d\n[DIV_Q_COMM] Rank: %d, Size: %d\n[MOD_Q_COMM] Rank: %d, Size: %d\n",
           rank, size, divRank, divSize, modRank, modSize);
    fflush(stdout); // Force text to print out to terminal immediately

    // Cleanup
    MPI_Finalize();
    if(rank == 0)
    {
        printf("\n");
    }

    return 0;
}
