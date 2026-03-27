
all: MPI

# --- Kokkos Targets ---

# --- MPI Targets ---

MPI: MPI_ParallelAdd

MPI_ParallelAdd: MPI_ParallelAdd.c
	mpicc MPI_ParallelAdd.c -o MPI_ParallelAdd.out

.PHONY: Run_MPI_ParallelAdd
Run_MPI_ParallelAdd:
	mpirun -np 4 ./MPI_ParallelAdd.out

# --- OpenMP Targets ---

# --- Utility Targets ---

.PHONY: clean
clean:
	rm MPI_ParallelAdd.out
