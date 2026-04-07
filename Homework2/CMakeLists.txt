cmake_minimum_required(VERSION 3.22)
project(Homework2)

# === Library Information ===
include(FetchContent)
FetchContent_Declare(
  Kokkos
  URL https://github.com/kokkos/kokkos/archive/refs/tags/5.0.0.zip
)
set(Kokkos_ENABLE_OPENMP ON)
FetchContent_MakeAvailable(Kokkos)

find_package(MPI REQUIRED)

# === Undergraduate Problems ===
add_executable(problem1 problem1.c)
target_link_libraries(problem1 MPI::MPI_C)

add_executable(problem2 problem2.cpp)
target_link_libraries(problem2 Kokkos::kokkos)

add_executable(problem3 problem3.cpp)
target_link_libraries(problem3 Kokkos::kokkos)

add_executable(problem4 problem4.cpp)
target_link_libraries(problem4 Kokkos::kokkos)

add_executable(problem5 problem5.cpp)
target_link_libraries(problem5 Kokkos::kokkos)

add_executable(problem6 problem6.cpp)
target_link_libraries(problem6 Kokkos::kokkos)

add_executable(problem7 problem7.cpp)
target_link_libraries(problem7 Kokkos::kokkos)

add_executable(problem8 problem8.c)
target_link_libraries(problem8 MPI::MPI_C)
