/**
 * Create a program that contains both a parallel for loop and a standard for
 * loop for summing rows of a View. Using Kokkos Timer, compare the performance
 * of the two for loops.
 */

#include <iomanip>
#include <iostream>

#include <Kokkos_Core.hpp>

//#define DEBUG_PRINT

static const int VIEW_HEIGHT = 10000;
static const int VIEW_WIDTH  = 100000;

void printSumView(const Kokkos::View <int *> &sumView)
{
    int i;
    for(i = 0; i < VIEW_HEIGHT; i++)
    {
        std::cout << std::setw(3) << sumView(i) << '\n';
    }
    std::cout << std::endl;
}

double parallelViewAdd(const Kokkos::View <int **> &srcView)
{
    // Perform parallel addition
    Kokkos::View <int *> sumView("sumView", VIEW_HEIGHT);
    Kokkos::Timer timer;
    Kokkos::parallel_for("parallelAdd", VIEW_HEIGHT, KOKKOS_LAMBDA(int l)
    {
        int k, rowSum;
        for(k = 0, rowSum = 0; k < VIEW_WIDTH; k++)
        {
            rowSum += srcView(l, k);
        }
        sumView(l) = rowSum;
    });

#ifdef DEBUG_PRINT
    std::cout << "Parallel Summation:\n";
    printSumView(sumView);
#endif
    return timer.seconds();
}

double linearViewAdd(const Kokkos::View <int **> &srcView)
{
    int i, j, sum;
    Kokkos::View <int *> sumView("sumView", VIEW_HEIGHT);
    Kokkos::Timer timer;
    for(i = 0; i < VIEW_HEIGHT; i++)
    {
        for(j = 0, sum = 0; j < VIEW_WIDTH; j++)
        {
            sum += srcView(i, j);
        }
        sumView(i) = sum;
    }

#ifdef DEBUG_PRINT
    std::cout << "Linear Summation:\n";
    printSumView(sumView);
#endif
    return timer.seconds();
}

int main()
{
    Kokkos::initialize();
    {
        int i, j;
        double parallelTime, linearTime;
        Kokkos::View <int **> myView("myView", VIEW_HEIGHT, VIEW_WIDTH);

        Kokkos::parallel_for("populateView", VIEW_HEIGHT, KOKKOS_LAMBDA(int l)
        {
            int k;
            for(k = 0; k < VIEW_WIDTH; k++)
            {
                myView(l, k) = l * k;
            }
        });

        #ifdef DEBUG_PRINT
            // Print view as vector to terminal
            std::cout << "View:\n";
            for(i = 0; i < VIEW_HEIGHT; i++)
            {
                for(j = 0; j < VIEW_WIDTH; j++)
                {
                    std::cout << std::setw(2) << myView(i, j) << " ";
                }
                std::cout << "\n";
            }
            std::cout << std::endl;
        #endif

        Kokkos::View <int *> sumView("sumView", VIEW_HEIGHT);

        parallelTime = parallelViewAdd(myView);
        linearTime   = linearViewAdd(myView);

        std::cout << "Parallel Time: " << parallelTime << " seconds\n"
                  << "Linear Time:   " << linearTime << " seconds" << std::endl;
    }

    // Clean up
    Kokkos::finalize();
    return 0;
}
