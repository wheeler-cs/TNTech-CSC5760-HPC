cmake_minimum_required(VERSION 3.22)
project(Homework2)

# === Library Information ===
include(FetchContent)
FetchContent_Declare(
  Kokkos
  URL https://github.com/kokkos/kokkos/archive/refs/tags/5.0.0.zip
)
FetchContent_MakeAvailable(Kokkos)

find_package(MPI REQUIRED)

# === Undergraduate Problems ===
add_executable(problem1 problem1.c)
target_link_libraries(problem1 MPI::MPI_C)

add_executable(problem2 problem2.cpp)
target_link_libraries(problem2 Kokkos::kokkos)
