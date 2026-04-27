#ifndef _PROBLEMG1_H
#define _PROBLEMG1_H

#include <stdlib.h>

#define DEBUG

#ifdef DEBUG
#define DBGPRINT(...) \
        printf("\n[DEBUG] "); \
        printf(__VA_ARGS__); \
        fflush(stdout);
#else
// Disable debug printing if undefined
#define DBGPRINT(...)
#endif

#define WORLD_WIDTH 79
#define WORLD_HEIGHT 49

// P * Q = Num of processes
#define P 2
#define Q 4

#define ITERATIONS 100

enum ReceiveTag
{
    TAG_NORTH,
    TAG_SOUTH,
    TAG_EAST,
    TAG_WEST,
    TAG_NORTHEAST,
    TAG_NORTHWEST,
    TAG_SOUTHEAST,
    TAG_SOUTHWEST,
};

enum WorldUpdateTag
{
    TAG_SUBWORLD_HEIGHT,
    TAG_SUBWORLD_WIDTH,
    TAG_SUBWORLD_ROW_ORIGIN,
    TAG_SUBWORLD_COL_ORIGIN,
    TAG_SUBWORLD_DATA,
};

enum CellState
{
    STATE_DEAD,
    STATE_ALIVE,
};

struct Allocations
{
    int rows,
        extraRows,
        cols,
        extraCols;
};

struct Allocations * initAllocationMap();
void deallocAllocationMap(struct Allocations *);


struct ProcessMap
{
    int map[P][Q];
};

struct ProcessMap * initProcMap();
void printProcMap(struct ProcessMap *);


struct NeighborRanks
{
    int n, s, e, w, ne, nw, se, sw, c;
};

struct NeighborRanks * calcNeighbors(int, struct ProcessMap *);
void printNeighborhood(struct NeighborRanks *);


struct ProcessChunkInfo
{
    int rowStart, rowEnd, rowRange,
        colStart, colEnd, colRange;
};

struct ProcessChunkInfo calcBoundaries(int, struct Allocations *);


struct ChunkHalos
{
    int * nHalo,
        * sHalo,
        * eHalo,
        * wHalo,
        * neHalo,
        * nwHalo,
        * seHalo,
        * swHalo;
    int nHaloSize,
        sHaloSize,
        eHaloSize,
        wHaloSize;
};

struct ChunkHalos * initHalos(int, int);
void deallocHalos(struct ChunkHalos *);
void exchangeHalos(int **, struct ChunkHalos *, struct NeighborRanks *);

void updateSubWorld(int **, struct ChunkHalos *, struct ProcessChunkInfo *);

void printSubworld(int **, struct ProcessChunkInfo *);

void aggregateSubWorlds(int [WORLD_HEIGHT][WORLD_WIDTH], struct ProcessMap *);
void forwardSubWorld(int **, struct ProcessChunkInfo *);
void applySubWorld(int [WORLD_HEIGHT][WORLD_WIDTH], int **, struct ProcessChunkInfo *);

void initWorld(int [WORLD_HEIGHT][WORLD_WIDTH]);
int isCellAlive(int, int, int **);
void blinkerDemo(int **);
void gliderDemo(int **);
void printWorld(int [WORLD_HEIGHT][WORLD_WIDTH]);


#endif
