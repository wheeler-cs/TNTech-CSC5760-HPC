/**
 * Link and run a program with Kokkos where you initialize a View and print out
 * its name with the .label() method.
 */

#include <iostream>

#include <Kokkos_Core.hpp>

int main() {
    Kokkos::initialize();
    {
        Kokkos::View<int **> myView ("My View", 2, 2);
        std::cout << myView.label() << std::endl;
    }

    // Clean up
    Kokkos::finalize();
    return 0;
}