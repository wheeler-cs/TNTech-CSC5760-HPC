/**
 * Declare a 5 × 7 × 12 × n View.
 */

#include <iostream>

#include <Kokkos_Core.hpp>

int main() {
    Kokkos::initialize();
    {
        int n;
        std::cout << "Enter value for n: ";
        std::cin >> n;
        Kokkos::View<int ****> myView ("My View", 5, 7, 12, n);
    }

    // Clean up
    Kokkos::finalize();
    return 0;
}