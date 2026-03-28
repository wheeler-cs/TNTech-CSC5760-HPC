CFLAGS=-Wall -Wextra -pedantic

all: problem1

# === Undergraduate Problems ===
problem1: problem1.c
	mpicc $(CFLAGS) problem1.c -o problem1

.PHONY: runp1
runp1:
	mpirun -np 4 ./problem1

# === Graduate Problems ===

# === Utility Targets ===
.PHONY: clean
clean:
	rm ./problem1
