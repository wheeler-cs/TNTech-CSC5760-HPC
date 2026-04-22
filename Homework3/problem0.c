/**
 * Imagine that you have a vector v, of length M. You have P processes, and the
 * vector is distributed among the processes twice—first as a linear mapping,
 * then as a scatter mapping. This exercise is about global-to-local and
 * local-to-global mappings in either linear or scattered distributions.
 * 
 * NOTE: In later problems, you will have such vectors distributed and
 * replicated on process topologies of shape P × Q, but here we are just
 * recalling how to map between local and global indices and processes in both
 * mappings
 */

#include <stdio.h>

#define P  4    //< Number of processes
#define M 10    //< Length of vector

/**
 * You have P processes, a vector v of length M, and you have the process rank
 * value as p, with index in that process i. Find the global index I of that
 * triple (p, i, M) with a linear inverse distribution.
 */
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
        // Calculate offset using mix of longer and shorter stide lengths
        globalIdx = ((baseStride + 1) * bonusElements);
        globalIdx += ((baseStride * (rank - bonusElements))) + procIdx;
    }

    return globalIdx;
}

/**
 * Given that same global index I, the length of the array M, and the number of
 * processes P, is it possible to find the rank of the process p, and the local
 * index i of its scatter-mapped index? How?
 */
int globalToLocalScatter(int globalIdx, int * rank)
{
    // Find owning rank for index
    *rank = globalIdx % P;

    // Calculate local index within rank
    return (globalIdx / P);
}

int main(int argc, char ** argv)
{
    int i, j;

    // Do local to global for linear distribution
    printf("Local-to-global conversion for linear distribution\n");
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
    printf("\n");

    // Do global to local for scatter distribution
    int rank, localIdx;
    printf("Global-to-local conversion for scatter distribution\n");
    for(i = 0; i < M; i++)
    {
        localIdx = globalToLocalScatter(i, &rank);
        printf("Rank: %d, Idx: %d\n", rank, localIdx);
    }

    return 0;
}
