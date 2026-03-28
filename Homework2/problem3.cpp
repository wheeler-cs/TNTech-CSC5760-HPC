/**
 * Make an n × m View where each index equals 1000 × i × j. Print out the view
 * as well.
 */

#include <iomanip>
#include <iostream>

#include <Kokkos_Core.hpp>


int main() {
    Kokkos::initialize();
    {
        int m, n, i, j;

        m = 10;
        n = 10;

        Kokkos::View<int **> myView ("My View", m, n);

        Kokkos::parallel_for("initializer", m, KOKKOS_LAMBDA(int i)
        {
            int j;
            for(j = 0; j < n; j++)
            {
                myView(i, j) = 1000 * i * j;
            }
        });

        for(i = 0; i < m; i++)
        {
            for(j = 0; j < n; j++)
            {
                std::cout << std::setw(5) << myView(i, j) << " ";
            }
            std::cout << std::endl;
        }
    }

    // Clean up
    Kokkos::finalize();
    return 0;
}