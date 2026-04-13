#ifndef ANSWER_SET_PARSER_H
#define ANSWER_SET_PARSER_H

#include <stdint.h>

typedef struct 
{
    int x, y, px, py;  // cell(x,y) connects to cell(x+px, y+py)
} parent;

typedef struct 
{
    int x,y;   //linear array 

}edge;

typedef enum 
{
    MAZE_FORMAT_PARENT, 
    MAZE_FORMAT_EDGE,

}mazeFormat;

typedef struct 
{
    int count;
    union 
    {
        parent *model;
        edge *edgeModels;

    }data;
} AnswerSetModels;

typedef struct 
{
    int count;
    mazeFormat f;
    AnswerSetModels *mods;
} AnswerSets;

typedef struct  
{
    AnswerSets *as;
    uint8_t answerSetNum;
    uint8_t currentAnswerSetRead;

    // dimensions of the maze described by the answer sets
    int mazeRows, mazeCols;
} answerSetsCtx;

// runs clingo with the given files, parses all answer sets, and returns a context.
answerSetsCtx* parseClingoOutput(const char *coreFile, const char *factsFile);


// runs clingo with the given files, parses all answer sets, and returns a context.
answerSetsCtx* parseClingoEdgeOutput(const char *coreFile, const char *factsFile);

// free
void freeAnswerSetCtx(answerSetsCtx *asc);

#endif
