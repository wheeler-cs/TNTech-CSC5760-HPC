#include <stdio.h>

#define P  4    //< Number of processes
#define M 10    //< Length of vector

int globalToLocalLinear(int rank, int globalIdx)
{
    // Function variables
    int baseStride,
        bonusElements,
        localIdx;
    baseStride = M / P;
    bonusElements = M % P;

    if(rank < bonusElements)
    {
        return 0;
    }
    return 0;
}

int localToGlobalLinear(int rank, int procIdx)
{
    // Function variables
    int baseStride,
        bonusElements,
        globalIdx;

    // Calculate standard stride and number of extra elements
    baseStride    = M / P;
    bonusElements = M % P;

    // Rank has longer stride
    if(rank < bonusElements)
    {
        globalIdx = (rank * (baseStride + 1)) + procIdx;
    }
    // Rank has smaller stride
    else
    {
        // Count all previous strides with extra elements
        globalIdx = ((baseStride + 1) * bonusElements);
        // Count all previous stides with no extra elements
        globalIdx += ((baseStride * (rank - bonusElements)));
        // Count offset of local process index
        globalIdx += procIdx;
    }

    return globalIdx;
}

int globalToLocalScatter(int rank, int globalIdx)
{
    return 0;
}

int localToGlobalScatter(int rank, int procIdx)
{
    return 0;
}

int main(int argc, char ** argv)
{
    int i, j;

    // Do local to global for linear distribution
    for(i = 0; i < P; i++)
    {
        printf("Rank %d: ", i);
        printf("%d ", localToGlobalLinear(i, 0));
        if(i < (M % P))
        {
            printf("%d ", localToGlobalLinear(i, 1));
            printf("%d\n", localToGlobalLinear(i, 2));
        }
        else
        {
            printf("%d\n", localToGlobalLinear(i, 1));
        }
    }

    return 0;
}
