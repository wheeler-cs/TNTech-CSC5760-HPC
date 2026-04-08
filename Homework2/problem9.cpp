/**
 * Using the Kokkos parallel scan function, perform a prefix sum on a 1D view
 * (of all 1s). Using the Kokkos timer, measure and print the time taken for the
 * parallel scan, and print the partial sums. Run this several times to compare
 * results between run.
 */

#include <iomanip>
#include <iostream>

#include <Kokkos_Core.hpp>

static const int VIEW_LEN = 10;

int main()
{
    Kokkos::initialize();
    {
        // Operation variables
        int i, sum;
        Kokkos::View <int *> myView("myView", VIEW_LEN);
        Kokkos::View <int *> prefixSumView("pfsView", VIEW_LEN);
        
        // Initialize view with all 1s
        Kokkos::parallel_for("initialize", VIEW_LEN, KOKKOS_LAMBDA(int i)
        {
            myView(i) = 1;
        });

        // Perform prefix sum
        Kokkos::parallel_scan("prefixSum", VIEW_LEN, KOKKOS_LAMBDA(int i, int &sum, bool final)
        {
            sum += myView(i);

            if(final)
            {
                prefixSumView(i) = sum;
            }
        }, sum);

        // Output prefix sum vector
        std::cout << "Output Prefix Sum: ";
        for(i = 0; i < VIEW_LEN; i++)
        {
            std::cout << std::setw(2) << prefixSumView(i) << " ";
        }
        std::cout << std::endl;
    }

    Kokkos::finalize();
    return 0;
}
