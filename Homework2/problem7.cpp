/**
 * Create a program that does adds the elements of a 1D View (a vector) to each
 * row of a 2D View (a matrix) with at least one loop of parallelism.
 * 
 * Extra credit (10pts/8pts): make a function and check for correct
 * shape/dimensions.
 */

#include <iomanip>
#include <iostream>

#include <Kokkos_Core.hpp>

//#define DEBUG_PRINT

int viewSizesCorrect(const Kokkos::View <int **> matrix, const Kokkos::View <int **> vector)
{
    return (matrix.extent(0) == vector.extent(1));
}

Kokkos::View <int **> addMatrixVector(const Kokkos::View <int **> matrix, const Kokkos::View <int **> vector)
{
    Kokkos::View <int **> tempSolution("tempSolution", 3, 3);
    Kokkos::parallel_for("matrixVectorAdd", 3, KOKKOS_LAMBDA(int i)
    {
        int j;
        for(j = 0; j < 3; j++)
        {
            tempSolution(i, j) = matrix(i, j) + vector(0, j);
        }
    });

    return tempSolution;
}

void printMatrix(const Kokkos::View <int **> matrix)
{
    int i, j;
    for(i = 0; i < matrix.extent(0); i++)
    {
        for(j = 0; j < matrix.extent(1); j++)
        {
            std::cout << std::setw(3) << matrix(i, j) << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

int main()
{
    Kokkos::initialize();
    {
        // Matrices and vectors are row major
        Kokkos::View <int **> matrix("matrix", 3, 3);
        Kokkos::View <int **> vector("vector", 1, 3);
        Kokkos::View <int **> solution("solution", 3, 3);

        // Check if vector and matrix are correct sizes
        if(viewSizesCorrect(matrix, vector))
        {
            // Populate matrix
            matrix(0, 0) = 130;
            matrix(1, 0) = 224;
            matrix(2, 0) =  54;
            matrix(0, 1) = 147;
            matrix(1, 1) = 158;
            matrix(2, 1) = 158;
            matrix(0, 2) = 115;
            matrix(1, 2) = 187;
            matrix(2, 2) = 120;
            // Populate vector
            vector(0, 0) = 221;
            vector(0, 1) =  12;
            vector(0, 2) = 157;

            std::cout << "Matrix A:\n";
            printMatrix(matrix);
            std::cout << "Matrix B:\n";
            printMatrix(vector);

            solution = addMatrixVector(matrix, vector);
            std::cout << "Solution:\n";
            printMatrix(solution);
        }
        else
        {
            std::cerr << "Vector and matrix are not appropriate sizes!" << std::endl;
        }
    }

    Kokkos::finalize();
    return 0;
}
