/** CODE GENERATED USING CHATGPT 5 FREE
 * 
 * PROMPT: Write a parallel vector-vector element by element multiplication program followed by a
 * tree reduction to compute the dot-product of two vectors of length N with P processes using MPI.
 * For instance, show with N = 1024, 2048, 4096 and P = 1, 2, 4. Use only point-to-point operations.
 * Code your own tree reduction using sends and receives. Divide the data as equally as you can
 * between your processes. We will discuss how to compute these partitions in class early next week
 * (week of February 16). Use MPI Wtime() to measure the cost of the operation as you vary P and N .
 * We will explain how to use this operation in lecture. Use easy-to-evaluate data for testing in
 * the vector elements for correctness testing (but not all zeroes).
 * 
 * LLM code has error handling for arguments passed into MPI. Additionally, it uses a function for
 * calculating the start and end of subvectors that are divided across multiple processes. There is
 * also the presences of barriers to force syncing across processes for certain operations are
 * performed. Structure of tree reductions are similar between the LLM code and my code.
 */


#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


void compute_partition(int N, int P, int rank, int *local_n, int *start);

int main(int argc, char *argv[]) {

    int rank, size;
    int N;
    double *x = NULL, *y = NULL;
    double local_sum = 0.0;
    double total_sum = 0.0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0)
            printf("Usage: mpirun -np P ./problem4 N\n");
        MPI_Finalize();
        return 0;
    }

    N = atoi(argv[1]);

    int local_n, start;
    compute_partition(N, size, rank, &local_n, &start);

    x = (double*) malloc(local_n * sizeof(double));
    y = (double*) malloc(local_n * sizeof(double));

    /* Initialize easy test data (non-zero, predictable) */
    for (int i = 0; i < local_n; i++) {
        int global_index = start + i;
        x[i] = global_index + 1;      // 1,2,3,...
        y[i] = 2.0 * (global_index + 1);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t_start = MPI_Wtime();

    /* Element-wise multiplication and local sum */
    for (int i = 0; i < local_n; i++) {
        local_sum += x[i] * y[i];
    }

    /* Manual tree reduction */
    int step = 1;
    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                double recv_val;
                MPI_Recv(&recv_val, 1, MPI_DOUBLE, rank + step, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                local_sum += recv_val;
            }
        } else {
            int dest = rank - step;
            MPI_Send(&local_sum, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
            break;
        }
        step *= 2;
    }

    double t_end = MPI_Wtime();

    if (rank == 0) {
        total_sum = local_sum;
        printf("N = %d, P = %d\n", N, size);
        printf("Dot product = %.2f\n", total_sum);
        printf("Time = %f seconds\n\n", t_end - t_start);
    }

    free(x);
    free(y);

    MPI_Finalize();
    return 0;
}

/* Compute start index and local size for each rank */
void compute_partition(int N, int P, int rank, int *local_n, int *start) {
    int base = N / P;
    int remainder = N % P;

    if (rank < remainder) {
        *local_n = base + 1;
        *start = rank * (*local_n);
    } else {
        *local_n = base;
        *start = remainder * (base + 1) + (rank - remainder) * base;
    }
}
