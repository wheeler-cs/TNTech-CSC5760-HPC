#include "problemg1.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

struct Allocations * initAllocationMap()
{
    // Allocate memory
    struct Allocations * allocMap;
    allocMap = malloc(sizeof(struct Allocations));

    // Calculate sizes
    allocMap->rows      = WORLD_HEIGHT / P;
    allocMap->extraRows = WORLD_HEIGHT % P;
    allocMap->cols      = WORLD_WIDTH  / Q;
    allocMap->extraCols = WORLD_WIDTH  % Q; 

    return allocMap;
}

void deallocAllocationMap(struct Allocations * allocMap)
{
    free(allocMap);
    allocMap = NULL;
}

struct ProcessMap * initProcMap()
{
    struct ProcessMap * pMap;
    pMap = malloc(sizeof(struct ProcessMap));

    int i, j;
    for(i = 0; i < P; i++)
    {
        for(j = 0; j < Q; j++)
        {
            pMap->map[i][j] = (i * Q) + j;
        }
    }

    return pMap;
}

void printProcMap(struct ProcessMap * pMap)
{
    int i, j;
    printf("\n");
    for(i = 0; i < P; i++)
    {
        for(j = 0; j < Q; j++)
        {
            printf("%d ", pMap->map[i][j]);
        }
        printf("\n");
    }
}

struct NeighborRanks * calcNeighbors(int rank, struct ProcessMap * pMap)
{
    int i, j, selfi, selfj;
    struct NeighborRanks * neighbors;

    // Find rank's position in process map
    for(i = 0; i < P; i++)
    {
        for(j = 0; j < Q; j++)
        {
            if(pMap->map[i][j] == rank)
            {
                selfi = i;
                selfj = j;
                i = P;
                break;
            }
        }
    }

    // Find neighboring processes (clockwise order from top left corner)
    neighbors = malloc(sizeof(struct NeighborRanks));
    neighbors->nw = pMap->map[((selfi + P) - 1) % P][((selfj + Q) - 1) % Q];
    neighbors->n  = pMap->map[((selfi + P) - 1) % P][selfj];
    neighbors->ne = pMap->map[((selfi + P) - 1) % P][(selfj + 1) % Q];
    neighbors->e  = pMap->map[selfi][(selfj + 1) % Q];
    neighbors->se = pMap->map[(selfi + 1) % P][(selfj + 1) % Q];
    neighbors->s  = pMap->map[(selfi + 1) % P][selfj];
    neighbors->sw = pMap->map[(selfi + 1) % P][((selfj + Q) - 1) % Q];
    neighbors->w  = pMap->map[selfi][((selfj + Q) - 1) % Q];

    neighbors->c = rank;

    return neighbors;
}

void printNeighborhood(struct NeighborRanks * neighbors)
{
    printf("\n%d %d %d", neighbors->nw, neighbors->n, neighbors->ne);
    printf("\n%d %d %d", neighbors->w, neighbors->c, neighbors->e);
    printf("\n%d %d %d", neighbors->sw, neighbors->s, neighbors->se);
    printf("\n");
}

struct ProcessChunkInfo calcBoundaries(int rank, struct Allocations * pMap)
{
    struct ProcessChunkInfo pChunkInfo;
    int procMapRow, procMapCol;
    procMapRow = rank / Q;
    procMapCol = rank % Q;

    // Calculate row and column range
    pChunkInfo.rowRange = pMap->rows;
    if(procMapRow < pMap->extraRows)
    {
        pChunkInfo.rowRange += 1;
    }
    pChunkInfo.colRange = pMap->cols;
    if(procMapCol < pMap->extraCols)
    {
        pChunkInfo.colRange += 1;
    }
    // Calculate row start
    pChunkInfo.rowStart = procMapRow * pMap->rows;
    if(procMapRow < pMap->extraRows)
    {
        pChunkInfo.rowStart += procMapRow;
    }
    else
    {
        pChunkInfo.rowStart += pMap->extraRows;
    }
    // Calculate column start
    pChunkInfo.colStart = procMapCol * pMap->cols;
    if(procMapCol < pMap->extraCols)
    {
        pChunkInfo.colStart += procMapCol;
    }
    else
    {
        pChunkInfo.colStart += pMap->extraCols;
    }
    // Calculate row and column end
    pChunkInfo.rowEnd = (pChunkInfo.rowStart + pChunkInfo.rowRange) % WORLD_HEIGHT;
    pChunkInfo.colEnd = (pChunkInfo.colStart + pChunkInfo.colRange) % WORLD_WIDTH;

    return pChunkInfo;
}

struct ChunkHalos * initHalos(int rowRange, int colRange)
{
    struct ChunkHalos * halos;
    halos = malloc(sizeof(struct ChunkHalos));

    // Set sizes of halos
    halos->eHaloSize = rowRange - 2;
    halos->wHaloSize = rowRange - 2;
    halos->nHaloSize = colRange - 2;
    halos->sHaloSize = colRange - 2;

    // Allocate memory for halos
    // Corners are 1-D arrays
    halos->eHalo  = malloc(sizeof(int) * halos->eHaloSize);
    halos->wHalo  = malloc(sizeof(int) * halos->wHaloSize);
    halos->nHalo  = malloc(sizeof(int) * halos->nHaloSize);
    halos->sHalo  = malloc(sizeof(int) * halos->sHaloSize);
    // Cardinal directions are scalars
    halos->neHalo = malloc(sizeof(int));
    halos->nwHalo = malloc(sizeof(int));
    halos->seHalo = malloc(sizeof(int));
    halos->swHalo = malloc(sizeof(int));

    return halos;
}

void deallocHalos(struct ChunkHalos * halos)
{
    free(halos->nHalo);
    free(halos->sHalo);
    free(halos->eHalo);
    free(halos->wHalo);
    free(halos->neHalo);
    free(halos->nwHalo);
    free(halos->seHalo);
    free(halos->swHalo);
    free(halos);
    halos = NULL;
}

void exchangeHalos(int ** subWorld, struct ChunkHalos * halos, struct NeighborRanks * neighbors)
{
    MPI_Request requests[16];
    int i;
    // Create buffers for sending data
    int * toNBuffer,
        * toSBuffer,
        * toEBuffer,
        * toWBuffer;
    toNBuffer = malloc(sizeof(int) * halos->nHaloSize);
    toSBuffer = malloc(sizeof(int) * halos->sHaloSize);
    toEBuffer = malloc(sizeof(int) * halos->eHaloSize);
    toWBuffer = malloc(sizeof(int) * halos->wHaloSize);

    // Initiate asynchornous receive first
    MPI_Irecv(halos->nHalo,  halos->nHaloSize, MPI_INT, neighbors->n,  TAG_NORTH,     MPI_COMM_WORLD, &requests[0]);
    MPI_Irecv(halos->sHalo,  halos->sHaloSize, MPI_INT, neighbors->s,  TAG_SOUTH,     MPI_COMM_WORLD, &requests[1]);
    MPI_Irecv(halos->eHalo,  halos->eHaloSize, MPI_INT, neighbors->e,  TAG_EAST,      MPI_COMM_WORLD, &requests[2]);
    MPI_Irecv(halos->wHalo,  halos->wHaloSize, MPI_INT, neighbors->w,  TAG_WEST,      MPI_COMM_WORLD, &requests[3]);
    MPI_Irecv(halos->neHalo, 1,                MPI_INT, neighbors->ne, TAG_NORTHEAST, MPI_COMM_WORLD, &requests[4]);
    MPI_Irecv(halos->nwHalo, 1,                MPI_INT, neighbors->nw, TAG_NORTHWEST, MPI_COMM_WORLD, &requests[5]);
    MPI_Irecv(halos->seHalo, 1,                MPI_INT, neighbors->se, TAG_SOUTHEAST, MPI_COMM_WORLD, &requests[6]);
    MPI_Irecv(halos->swHalo, 1,                MPI_INT, neighbors->sw, TAG_SOUTHWEST, MPI_COMM_WORLD, &requests[7]);

    // Copy data to send from chunk to buffers
    for(i = 0; i < halos->nHaloSize; i++)
    {
        toNBuffer[i] = subWorld[0][i + 1];
    }
    for(i = 0; i < halos->sHaloSize; i++)
    {
        toSBuffer[i] = subWorld[halos->eHaloSize - 1][i];
    }
    for(i = 0; i < halos->eHaloSize; i++)
    {
        toEBuffer[i] = subWorld[i][halos->sHaloSize];
    }
    for(i = 0; i < halos->wHaloSize; i++)
    {
        toWBuffer[i] = subWorld[i][0];
    }

    // Send data to neighbors; NOTE: Tag is opposite direction of where halo goes
    // Send cardinal halos
    MPI_Isend(toNBuffer, halos->nHaloSize, MPI_INT, neighbors->n, TAG_SOUTH, MPI_COMM_WORLD, &requests[8]);
    MPI_Isend(toSBuffer, halos->sHaloSize, MPI_INT, neighbors->s, TAG_NORTH, MPI_COMM_WORLD, &requests[9]);
    MPI_Isend(toEBuffer, halos->eHaloSize, MPI_INT, neighbors->e, TAG_WEST,  MPI_COMM_WORLD, &requests[10]);
    MPI_Isend(toWBuffer, halos->wHaloSize, MPI_INT, neighbors->w, TAG_EAST,  MPI_COMM_WORLD, &requests[11]);
    // Send corners
    MPI_Isend(&subWorld[1][1],                                       1, MPI_INT, neighbors->nw, TAG_SOUTHEAST, MPI_COMM_WORLD, &requests[12]);
    MPI_Isend(&subWorld[1][halos->nHaloSize],                    1, MPI_INT, neighbors->ne, TAG_SOUTHWEST, MPI_COMM_WORLD, &requests[13]);
    MPI_Isend(&subWorld[halos->eHaloSize][halos->nHaloSize], 1, MPI_INT, neighbors->se, TAG_NORTHWEST, MPI_COMM_WORLD, &requests[14]);
    MPI_Isend(&subWorld[halos->eHaloSize][0],                    1, MPI_INT, neighbors->sw, TAG_NORTHEAST, MPI_COMM_WORLD, &requests[15]);

    // Wait for send to finish
    MPI_Waitall(16, requests, MPI_STATUSES_IGNORE);

    // Free dynamic memory
    free(toNBuffer);
    free(toSBuffer);
    free(toEBuffer);
    free(toWBuffer);
}

void updateSubWorld(int ** subWorld, struct ChunkHalos * halos, struct ProcessChunkInfo * pcInfo)
{
    // Function variables
    int i, j;
    int ** worldCopy;
    
    // Allocate memory for temporary world
    worldCopy = malloc(sizeof(int *) * (pcInfo->rowRange + 2));

    for(i = 0; i < pcInfo->rowRange + 2; i++)
    {
        worldCopy[i] = malloc(sizeof(int) * (pcInfo->colRange + 2));
    }

    // Copy corner halos to subworld
    subWorld[0][0] = *(halos->nwHalo);                                       // NW
    subWorld[0][pcInfo->colRange + 1] = *(halos->neHalo);                    // NE
    subWorld[pcInfo->rowRange + 1][0] = *(halos->swHalo);                    // SW
    subWorld[pcInfo->rowRange + 1][pcInfo->colRange + 1] = *(halos->seHalo); // SE
    // Copy cells from cardinal direction halos
    for(i = 0; i < halos->nHaloSize; i++) // North
    {
        subWorld[0][i + 1] = halos->nHalo[i];
    }
    for(i = 0; i < halos->sHaloSize; i++) // South
    {
        subWorld[pcInfo->rowRange + 1][i + 1] = halos->sHalo[i];
    }
    for(i = 0; i < halos->eHaloSize; i++) // East
    {
        subWorld[i + 1][pcInfo->colRange + 1] = halos->eHalo[i];
    }
    for(i = 0; i < halos->wHaloSize; i++) // West
    {
        subWorld[i + 1][0] = halos->wHalo[i];
    }

    // Perform actual update on world
    for(i = 0; i < pcInfo->rowRange; i++)
    {
        for(j = 0; j < pcInfo->colRange; j++)
        {
            worldCopy[i + 1][j + 1] = isCellAlive(i + 1, j + 1, subWorld);
        }
    }

    // Transfer copy to real world
    for(i = 1; i < pcInfo->rowRange + 1; i++)
    {
        for(j = 1; j < pcInfo->colRange + 1; j++)
        {
            subWorld[i][j] = worldCopy[i][j];
        }
    }

    // Cleanup
    for(i = 0; i < pcInfo->rowRange + 2; i++)
    {
        free(worldCopy[i]);
        worldCopy[i] = NULL;
    }
    free(worldCopy);
    worldCopy = NULL;
}

void printSubworld(int ** subWorld, struct ProcessChunkInfo * pcInfo)
{
    int i, j;
    printf("\n\n");
    for(j = pcInfo->rowRange + 1; j >= 0; j--)
    {
        for(i = 0; i < pcInfo->colRange + 2; i++)
        {
            if(subWorld[j][i] == STATE_ALIVE)
            {
                printf("*");
            }
            else
            {
                printf("-");
            }
        }
        printf("\n");
    } 
}

void aggregateSubWorlds(int world[WORLD_HEIGHT][WORLD_WIDTH], struct ProcessMap * pMap)
{
    int i, j, k, l,
        subWorldRowOrigin, subWorldColOrigin,
        subWorldHeight, subWorldWidth,
        * tempSubWorld;
    for(i = 0; i < P; i++)
    {
        for(j = 0; j < Q; j++)
        {
            if(pMap->map[i][j] == 0)
            {
                continue;
            }
            MPI_Recv(&subWorldRowOrigin, 1, MPI_INT, pMap->map[i][j], TAG_SUBWORLD_ROW_ORIGIN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&subWorldColOrigin, 1, MPI_INT, pMap->map[i][j], TAG_SUBWORLD_COL_ORIGIN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Force blocking receive for height and width to know how big world is
            MPI_Recv(&subWorldHeight, 1, MPI_INT, pMap->map[i][j], TAG_SUBWORLD_HEIGHT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&subWorldWidth, 1, MPI_INT, pMap->map[i][j], TAG_SUBWORLD_WIDTH, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Allocate memory for map and receive data
            tempSubWorld = malloc(sizeof(int) * subWorldHeight * subWorldWidth);
            MPI_Recv(tempSubWorld, subWorldHeight * subWorldWidth, MPI_INT, pMap->map[i][j], TAG_SUBWORLD_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Copy over world data
            for(k = 0; k < subWorldHeight; k++)
            {
                for(l = 0; l < subWorldWidth; l++)
                {
                    world[subWorldRowOrigin + k][subWorldColOrigin + l] = tempSubWorld[(k * subWorldWidth) + l];
                }
            }
            free(tempSubWorld);
        }
    }
}

void forwardSubWorld(int ** subWorld, struct ProcessChunkInfo * pcInfo)
{
    MPI_Request requests[5];
    int i, j,
      * flatWorld;

    // Flatten subworld into contiguous memory
    flatWorld = malloc(sizeof(int) * (pcInfo->rowRange) * (pcInfo->colRange));
    for(i = 1; i < pcInfo->rowRange; i++)
    {
        for(j = 1; j < pcInfo->colRange; j++)
        {
            flatWorld[((i - 1) * pcInfo->colRange) + (j - 1)] = subWorld[i][j];
        }
    }

    // Send all data at once and await for completion
    MPI_Isend(&(pcInfo->rowStart), 1, MPI_INT, 0, TAG_SUBWORLD_ROW_ORIGIN, MPI_COMM_WORLD, &requests[0]);
    MPI_Isend(&(pcInfo->colStart), 1, MPI_INT, 0, TAG_SUBWORLD_COL_ORIGIN, MPI_COMM_WORLD, &requests[1]);
    MPI_Isend(&(pcInfo->rowRange), 1, MPI_INT, 0, TAG_SUBWORLD_HEIGHT, MPI_COMM_WORLD, &requests[2]);
    MPI_Isend(&(pcInfo->colRange), 1, MPI_INT, 0, TAG_SUBWORLD_WIDTH, MPI_COMM_WORLD, &requests[3]);
    MPI_Isend(flatWorld, (pcInfo->rowRange) * (pcInfo->colRange), MPI_INT, 0, TAG_SUBWORLD_DATA, MPI_COMM_WORLD, &requests[4]);
    MPI_Waitall(5, requests, MPI_STATUSES_IGNORE);

    // Cleanup
    free(flatWorld);
}

void applySubWorld(int world[WORLD_HEIGHT][WORLD_WIDTH], int ** subWorld, struct ProcessChunkInfo * pcInfo)
{
    int i, j;
    for(i = 1; i < pcInfo->rowRange; i++)
    {
        for(j = 1; j < pcInfo->colRange; j++)
        {
            world[i + pcInfo->rowStart][j + pcInfo->colStart] = subWorld[i][j];
        }
    }
}


void initWorld(int world[WORLD_HEIGHT][WORLD_WIDTH])
{
    int i, j;
    for(i = 0; i < WORLD_HEIGHT; i++)
    {
        for(j = 0; j < WORLD_WIDTH; j++)
        {
            world[i][j] = STATE_DEAD;
        }
    }
}

int isCellAlive(int r, int c, int ** world)
{
    int neighbors, i, j, neighborRow, neighborCol;
    neighbors = 0;
    // Check 3 x 3 box around 
    for(i = -1; i <= 1; i++)
    {
        for(j = -1; j <= 1; j++)
        {
            // Skip over self
            if((i == 0) && (j == 0))
            {
                continue;
            }
            // Check cell state
            neighborRow = (r + i);
            neighborCol = (c + j);
            if(world[neighborRow][neighborCol] == STATE_ALIVE)
            {
                neighbors++;
            }
        }
    }
    if(world[r][c] == STATE_ALIVE)
    {
        // If alive @ t, is alive @ t + 1 if has 2 or 3 neighbors
        if(neighbors != 2 && neighbors != 3)
        {
            return STATE_DEAD;
        }
        else
        {
            return STATE_ALIVE;
        }
    }
    else
    {
        // If dead @ t, is alive @ t + 1 if has 3 neighbors
        if(neighbors == 3)
        {
            return STATE_ALIVE;
        }
        else
        {
            return STATE_DEAD;
        }
    }
}

void blinkerDemo(int ** world)
{
    DBGPRINT("Setting up blinker demo")
    world[2][2] = STATE_ALIVE;
    world[2][3] = STATE_ALIVE;
    world[2][4] = STATE_ALIVE;
}

void gliderDemo(int ** world)
{
    DBGPRINT("Setting up glider demo")
    world[0][0] = STATE_ALIVE;
    world[0][1] = STATE_ALIVE;
    world[0][2] = STATE_ALIVE;
    world[1][2] = STATE_ALIVE;
    world[2][0] = STATE_ALIVE;
}

void printWorld(int world[WORLD_HEIGHT][WORLD_WIDTH])
{
    int i, j;
    printf("\n");
    for(j = WORLD_HEIGHT - 1; j >= 0; j--)
    {
        for(i = 0; i < WORLD_WIDTH; i++)
        {
            if(world[j][i] == STATE_ALIVE)
            {
                printf("*");
            }
            else
            {
                printf("-");
            }
        }
        printf("\n");
    }
    fflush(stdout);
}
