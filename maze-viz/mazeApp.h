#ifndef MAZE_APP_H
#define MAZE_APP_H

#include <stdbool.h>
#include <stdint.h>
#include "answerSetParser.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    NORTH = 1 << 0,
    EAST  = 1 << 1,
    SOUTH = 1 << 2,
    WEST  = 1 << 3
} direction;

typedef struct
{
    int from;
    int to;
} PathEdge;

typedef struct
{
    int count;
    int cap;
    PathEdge *edges;
} PathModel;

typedef struct
{
    int count;
    PathModel *mods;
} PathSet;

int writeMazeEdgesToFile(const char *filename, const uint8_t *cellWalls,
                         int rows, int cols);

int solveMaze(const uint8_t *maze,  int rows, 
                                    int cols,
                                    const char *solverFile, 
                                    PathModel *out);

PathSet *solveAllMazes(uint8_t **mazes, int mazeCount, 
                                        int rows, 
                                        int cols,
                                        const char *solverFile);

void freePathSet(PathSet *paths);

//on‑demand text generation
char* generateEdgesString(const uint8_t *cellWalls, int rows, int cols);
char* runSolverOnMaze(const uint8_t *maze, int rows, 
                                           int cols,
                                           const char *solverFile);

int runApp(uint8_t **mazes, PathSet *paths, 
                            AnswerSets *parentSets,
                            int mazeCount, 
                            int rows, 
                            int cols);

//a dump folder for the relavant files :TODO
int dumpAllMazeEdgeFiles(const char *dirName,
                         uint8_t **mazes,
                         int mazeCount,
                         int rows,
                         int cols);

#ifdef __cplusplus
}
#endif

#endif
