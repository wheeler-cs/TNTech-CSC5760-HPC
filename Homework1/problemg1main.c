// Main header
#include "problemg1.h"

// Terminal utilities (for prettier printing)
#include "Terminal.h"

// Library includes
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Needed for sleep()

/* I'm not normally one for griping about an assignment, but I think something
 * needs to be said about this one. I think the difficulty of this homework
 * does not accurately reflect the fact that it is the first one of the
 * semester; that is, I believe that it was inappropriate for the level of
 * knowledge we currently have. Regardless of the fact that this is for the
 * graduate-level section of this course, just about everyone in this class is
 * still new to the concepts of parallel computing, and MPI is still a brand
 * new library to us. Me, personally, I've had very limited experience with
 * pthreads, and that's the extent of my knowledge about parallel computing.
 * 
 * I genuinely put my best effort into writing this program, but I've had course
 * semester projects with fewer lines of source code than what I've put in here.
 * I just wish there had been a more gradual introduction to MPI, and that the
 * lectures had gone into more detail about what each function does and how it
 * works.Some smaller code examples that we could pull from would also have been
 * helpful. While the examples provided do have valuable information, they are
 * monolithic and pretty intimidating to break down.
 * 
 * Again, I'm typically not the person who complains about an assignment, but
 * I think this was excessive for the first one of the semester.
 */

int main(int argc, char ** argv)
{
    int world[WORLD_HEIGHT][WORLD_WIDTH];
    initWorld(world);

    // MPI Initialization
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Calculate how many extra rows and columns there are
    struct Allocations * allocMap;
    allocMap = initAllocationMap();
    
    // Map processes to world
    int i, j;
    struct ProcessMap * pMap;
    pMap = initProcMap();

    // Map process neighbors
    struct NeighborRanks * neighbors;
    neighbors = calcNeighbors(rank, pMap);

    // Calculate sub-world chunk sizes and allocate
    struct ProcessChunkInfo pcInfo;
    int ** subWorld;
    pcInfo = calcBoundaries(rank, allocMap);
    // Allocate memory for local world chunk
    subWorld = malloc(sizeof(int *) * (pcInfo.rowRange + 2));
    for(i = 0; i < pcInfo.rowRange + 2; i++)
    {
        subWorld[i] = malloc(sizeof(int) * (pcInfo.colRange + 2));
    }
    // Blank-init elements in local world
    for(i = 0; i < pcInfo.rowRange + 2; i++)
    {
        for(j = 0; j < pcInfo.colRange + 2; j++)
        {
            subWorld[i][j] = STATE_DEAD;
        }
    }

    // Setup halos for data from other processes
    struct ChunkHalos * halos;
    halos = initHalos(pcInfo.rowRange, pcInfo.colRange);

    // Init worlds with data
    if(rank == 0)
    {
        blinkerDemo(subWorld);
    }
    else
    {
        for(i = 1; i < pcInfo.rowRange + 1; i++)
        {
            for(j = 1; j < pcInfo.colRange + 1; j++)
            {
                if(rank % 2)
                {
                    subWorld[i][j] = STATE_ALIVE;
                }
                else
                {
                    subWorld[i][j] = STATE_DEAD;
                }
            }
        }
    }
    // Run game for n iterations
    for(i = 0; i < ITERATIONS; i++)
    {
        exchangeHalos(subWorld, halos, neighbors);
        //if(rank == 2)
        //{
        //    printSubworld(subWorld, &pcInfo);
        //}
        updateSubWorld(subWorld, halos, &pcInfo);

        // Update world if rank 0
        if(rank == 0)
        {
            aggregateSubWorlds(world, pMap);
            applySubWorld(world, subWorld, &pcInfo);
            clearScreen();
            setCursorPosition(1, 1);
            DBGPRINT("Printing iteration [%d]", i + 1)
            printWorld(world);
        }
        // All other ranks forward world state to 0
        else
        {
            forwardSubWorld(subWorld, &pcInfo);
        }
        sleep(1); // Sleep so user can see updates in terminal
    }

    // Cleanup
    deallocAllocationMap(allocMap);
    deallocHalos(halos);
    for(i = 0; i < pcInfo.rowRange; i++)
    {
        free(subWorld[i]);
    }
    free(subWorld);
    free(pMap);
    free(neighbors);
    MPI_Finalize();
    if(rank == 0)
    {
        printf("\n");
    }

    return 0;
}
