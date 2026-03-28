/**
 * Do simple parallel reduce to output (print) the maximum element in a View.
 * Please do not populate the view with random elements. 
 */


#include <iomanip>
#include <iostream>

#include <Kokkos_Core.hpp>

//#define DEBUG_PRINT

static const int VECTOR_LEN = 1000000000;

int main() {
    Kokkos::initialize();
    {
        int i, maxVal;
        maxVal = 0;

        Kokkos::View <int *> myView("My View", VECTOR_LEN);

        // Populate view with values in range [0, VECTOR_LEN)
        Kokkos::parallel_for("initializer", VECTOR_LEN, KOKKOS_LAMBDA(int i)
        {
                myView(i) = i;
        });

    #ifdef DEBUG_PRINT
        // Print view as vector to terminal
        std::cout << "Vector: ";
        for(i = 0; i < VECTOR_LEN; i++)
        {
            std::cout << std::setw(2) << myView(i) << " ";
        }
        std::cout << std::endl;
    #endif

        // Perform parallel reduction to find max value
        Kokkos::parallel_reduce("parallelMax", myView.extent(0), KOKKOS_LAMBDA(const int &i, int &val)
        {
            if(myView(i) > val)
            {
                val = myView(i);
            }
        },
        Kokkos::Max<int>(maxVal));
        std::cout << "Max Value: " << maxVal << std::endl;
    }

    // Clean up
    Kokkos::finalize();
    return 0;
}