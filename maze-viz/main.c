#include "mazeApp.h"
#include "answerSetParser.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline int getIndexFromRowCol(int rows, int cols, int r, int c)
{
    return r * cols + c;
}


typedef struct
{
    uint8_t **mazes;
    int currMaze;
    int maxMaze;
} Maze;


static Maze*
computeMazes(answerSetsCtx *asc)
{
    if (!asc || !asc->as) return NULL;

    Maze *mazeNode = malloc(sizeof(Maze));
    if (!mazeNode) return NULL;

    mazeNode->mazes = malloc(asc->answerSetNum * sizeof(uint8_t *));
    if (!mazeNode->mazes) 
    {
        free(mazeNode);
        return NULL;
    }

    for (int k = 0; k < asc->answerSetNum; k++) {
        mazeNode->mazes[k] = calloc(asc->mazeRows * asc->mazeCols, sizeof(uint8_t));
        if (!mazeNode->mazes[k]) 
        {
            // cleanup prev allocations
            for (int j = 0; j < k; j++) free(mazeNode->mazes[j]);
            free(mazeNode->mazes);
            free(mazeNode);
            return NULL;
        }

        // start with a maze with all walls present
        for (int i = 0; i < asc->mazeRows * asc->mazeCols; i++)
            mazeNode->mazes[k][i] = NORTH | EAST | SOUTH | WEST;

        // passage using answer sets 
        AnswerSetModels *mod = &asc->as->mods[k];
        for (int i = 0; i < mod->count; i++) 
        {
            parent p = mod->data.model[i];
            
            // connect cell (p.x, p.y) to its parent (p.x + p.px, p.y + p.py)
            int px = p.x + p.px;
            int py = p.y + p.py;
            if (px < 0 || px >= asc->mazeRows || py < 0 || py >= asc->mazeCols) continue;

            int idx = getIndexFromRowCol(asc->mazeRows, asc->mazeCols, p.x, p.y);
            int parent_idx = getIndexFromRowCol(asc->mazeRows, asc->mazeCols, px, py);

            if (px == p.x - 1 && py == p.y) { // parent above
                mazeNode->mazes[k][idx] &= ~NORTH;
                mazeNode->mazes[k][parent_idx] &= ~SOUTH;
            } else if (px == p.x + 1 && py == p.y) { // parent below
                mazeNode->mazes[k][idx] &= ~SOUTH;
                mazeNode->mazes[k][parent_idx] &= ~NORTH;
            } else if (px == p.x && py == p.y - 1) { // parent left
                mazeNode->mazes[k][idx] &= ~WEST;
                mazeNode->mazes[k][parent_idx] &= ~EAST;
            } else if (px == p.x && py == p.y + 1) { // parent right
                mazeNode->mazes[k][idx] &= ~EAST;
                mazeNode->mazes[k][parent_idx] &= ~WEST;
            }
        }
    }

    return mazeNode;
}

static Maze *computeMazesFromEdges(answerSetsCtx *asc)
{
    if (!asc || !asc->as) return NULL;
    if (asc->mazeRows <= 0 || asc->mazeCols <= 0) return NULL;

    Maze *mazeNode = malloc(sizeof(Maze));
    if (!mazeNode) return NULL;

    mazeNode->mazes = malloc(asc->answerSetNum * sizeof(uint8_t *));
    if (!mazeNode->mazes) {
        free(mazeNode);
        return NULL;
    }

    for (int k = 0; k < asc->answerSetNum; k++) {
        mazeNode->mazes[k] = calloc(asc->mazeRows * asc->mazeCols, sizeof(uint8_t));
        if (!mazeNode->mazes[k]) {
            for (int j = 0; j < k; j++)
                free(mazeNode->mazes[j]);
            free(mazeNode->mazes);
            free(mazeNode);
            return NULL;
        }

        // start with all walls present 
        for (int i = 0; i < asc->mazeRows * asc->mazeCols; i++)
            mazeNode->mazes[k][i] = NORTH | EAST | SOUTH | WEST;

        AnswerSetModels *mod = &asc->as->mods[k];

        for (int i = 0; i < mod->count; i++) {
            edge e = mod->data.edgeModels[i];

            int a = e.x; 
            int b = e.y; 

            if (a < 0 || b < 0) continue;
            if (a >= asc->mazeRows * asc->mazeCols) continue;
            if (b >= asc->mazeRows * asc->mazeCols) continue;

            int ar = a / asc->mazeCols;
            int ac = a % asc->mazeCols;
            int br = b / asc->mazeCols;
            int bc = b % asc->mazeCols;

            int idxA = getIndexFromRowCol(asc->mazeRows, asc->mazeCols, ar, ac);
            int idxB = getIndexFromRowCol(asc->mazeRows, asc->mazeCols, br, bc);

            //check if they are neighbours and make a passage 
            if (ar == br && bc == ac + 1) 
            {
            
                mazeNode->mazes[k][idxA] &= ~EAST;  //B to the right of A
                mazeNode->mazes[k][idxB] &= ~WEST;
            }
            else if (ar == br && bc == ac - 1) 
            {
                mazeNode->mazes[k][idxA] &= ~WEST;  // B to the left of A
                mazeNode->mazes[k][idxB] &= ~EAST;
            }
            else if (ac == bc && br == ar + 1) 
            {
                mazeNode->mazes[k][idxA] &= ~SOUTH;  //B below A
                mazeNode->mazes[k][idxB] &= ~NORTH;
            }
            else if (ac == bc && br == ar - 1) 
            {
                mazeNode->mazes[k][idxA] &= ~NORTH;  //B aboVE S
                mazeNode->mazes[k][idxB] &= ~SOUTH;
            }
        }
    }

    return mazeNode;
}


//write facts for  gen1
void writeFactsFile(int dim)
{
    FILE *facts = fopen("facts.lp", "w");
    if (!facts) 
    {
        perror("fopen(facts.lp)");
        exit(1);
    }

    fprintf(facts, "#const width = %d.\n", dim);
    fprintf(facts, "dim(1..width).\n");
    fclose(facts);
}


//write facts for gen0
void writeFactsFileEdges(int dim)
{
    FILE *facts = fopen("facts.lp", "w");
    if (!facts) 
    {
        perror("fopen(facts.lp)");
        exit(1);
    }

    fprintf(facts, "#const cols = %d.\n", dim);
    fprintf(facts, "#const rows = %d.\n", dim);
    fprintf(facts, "size(%d,%d).\n", dim, dim);
    fprintf(facts, "start(1,1).\n");
    fprintf(facts, "end(%d,%d).\n", dim ,dim);
    fclose(facts);
}

int main(int argc, char *argv[])
{
    int gen = 0;

    //parse the command line arguments 
    /*This was meant for toggling between different maze generation encodings
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--g") == 0 && i+1 < argc)
        {
            if (strcmp(argv[i+1], "gen0") == 0)
                gen = 0;
            else if (strcmp(argv[i+1], "gen1") == 0)
                gen = 1;
            else 
            {
                fprintf(stderr, "Unknown generator: %s. Use 'gen0' or 'gen1'.\n", argv[i+1]);
                return 1;
            }
            i++;
        }     
    }
*/
    int dim;
    printf("Enter the length of your maze (default=5): ");
    scanf("%d", &dim);
    if (dim < 1) dim = 5;

    if(gen == 0) writeFactsFileEdges(dim); else writeFactsFile(dim);

    answerSetsCtx *asc;

    asc = gen == 0 ? parseClingoEdgeOutput("maze.lp","facts.lp") : parseClingoOutput("maze-core.lp", "facts.lp");
    if (!asc || asc->answerSetNum == 0) {
        fprintf(stderr, "No answer sets generated.\n");
        return 1;
    }

FILE *orig = fopen("original_edges.lp", "w");
for (int i = 0; i < asc->answerSetNum; i++) 
{
    AnswerSetModels *mod = &asc->as->mods[i];
    for (int j = 0; j < mod->count; j++) 
    {
        edge e = mod->data.edgeModels[j];
        // convert back to 1‑based indices
        fprintf(orig, "edge(%d,%d).\n", e.x+1, e.y+1);
    }
}
fclose(orig);

    asc->mazeRows = dim;
    asc->mazeCols = dim;

    Maze *mazes = gen == 0 ? computeMazesFromEdges(asc) :  computeMazes(asc);
    if (!mazes) {
        fprintf(stderr, "Failed to compute mazes.\n");
        freeAnswerSetCtx(asc);
        return 1;
    }



    PathSet *paths = solveAllMazes(mazes->mazes, asc->answerSetNum,
            asc->mazeRows, asc->mazeCols,
            "maze_solver.lp");

    // pass asc->as (parent atoms) to runApp
    runApp(mazes->mazes, paths, asc->as,
            asc->answerSetNum, asc->mazeRows, asc->mazeCols);

    freePathSet(paths);
    for (int i = 0; i < asc->answerSetNum; i++) free(mazes->mazes[i]);
    free(mazes->mazes);
    free(mazes);
    freeAnswerSetCtx(asc);
    return 0;
}
