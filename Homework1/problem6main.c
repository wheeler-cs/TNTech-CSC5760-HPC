#include <mpi.h>
#include <stdio.h>

int main(int argc, char ** argv)
{
    // MPI startup
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Setup variables for calculation
    int val, result;
    val = 1;

    // Fake MPI_Allreduce
    MPI_Reduce(&val,
               &result,
               1,
               MPI_INT,
               MPI_SUM,
               0,
               MPI_COMM_WORLD);
    MPI_Bcast(&result,
              1,
              MPI_INT,
              0,
              MPI_COMM_WORLD);
    printf("[MPI_Allreduce] Rank: %d, Result: %d", rank, result);

    // Fake MPI_Allgather
    int vals[8];
    MPI_Gather(&val,
               1,
               MPI_INT,
               vals,
               1,
               MPI_INT,
               0,
               MPI_COMM_WORLD);
    MPI_Bcast(vals,
              size,
              MPI_INT,
              0,
              MPI_COMM_WORLD);
    printf("\n[MPI_Allgather] Rank: %d, Result: [ ", rank);
    int i;
    for(i = 0; i < size; i++)
    {
        printf("%d ", vals[i]);
    }
    printf("]");

    // MPI_Alltoall
    int send[8], recv[8];
    for(i = 0; i < 8; i++)
    {
        send[i] = i;
    }
    MPI_Alltoall(send,
                 1,
                 MPI_INT,
                 recv,
                 1,
                 MPI_INT,
                 MPI_COMM_WORLD);
    printf("\n[MPI_Alltoall] Rank: %d, Data: [ ", rank);
    for(i = 0; i < size; i++)
    {
        printf("%d ", recv[i]);
    }
    printf("]");

    // Cleanup
    MPI_Finalize();
    printf("\n");
    return 0;
}
