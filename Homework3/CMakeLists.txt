cmake_minimum_required(VERSION 3.22)
project(Homework3 LANGUAGES C CUDA)

# === Library Imports ===

find_package(MPI REQUIRED)


# === Undergraduate Problems ===

add_executable(problem0 problem0.c)

add_executable(problem1 problem1.c)
target_link_libraries(problem1 MPI::MPI_C)

add_executable(problem2 problem2.c)
target_link_libraries(problem2 MPI::MPI_C)

add_executable(problem4 problem4.cu)
set_target_properties(problem4 PROPERTIES
                      CUDA_STANDARD 17
                      CUDA_STANDARD_REQUIRED ON
                      CUDA_ARCHITECTURES native)

# === Graduate Problems ===
