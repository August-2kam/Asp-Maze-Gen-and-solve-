#include "answerSetParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


//parse the clingo output into an internal data struct - parent(_,_,_,_)
static parent getParent(char *atom)
{
    parent p = {0};
    if (!atom) return p;

    // skip space
    while (isspace((unsigned char)*atom)) atom++;

    static const char prefix[] = "parent(";
    size_t prefixSize = sizeof(prefix) - 1;
    if (strncmp(atom, prefix, prefixSize) != 0) return p;

    const char *s = atom + prefixSize;
    int x, y, px, py, n = 0;
    if (sscanf(s, "%d,%d,%d,%d %n", &x, &y, &px, &py, &n) < 4)
        return p;
    s += n;
    while (isspace((unsigned char)*s)) s++;
    if (*s != ')') return p;

    // 0 based indeces 
    p.x = x - 1;
    p.y = y - 1;
    p.px = px;
    p.py = py;
    return p;
}


//parse the clingo output into an internal data struct - edge(_,_)
static edge getEdge(char *atom)
{
    edge e = { -1, -1 };
    if (!atom) return e;

    while (isspace((unsigned char)*atom)) atom++;

    int a, b;
    if (sscanf(atom, "edge(%d,%d).", &a, &b) == 2 ||
        sscanf(atom, "edge(%d,%d)",  &a, &b) == 2)
    {
        e.x = a - 1;
        e.y = b - 1;
    }

    return e;
}

static void 
freeAnswerSets(AnswerSets *sets)
{
    if (!sets) return;
    for (int i = 0; i < sets->count; i++)
    {
        if(sets->f ==MAZE_FORMAT_PARENT)
           free(sets->mods[i].data.model); 
        else free(sets->mods[i].data.edgeModels);
    }    
    free(sets->mods);
    free(sets);
}


//parse clingo file 
answerSetsCtx* 
parseClingoOutput(const char *coreFile, const char *factsFile)
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "clingo %s %s -n 200", coreFile, factsFile);
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen(clingo)");
        return NULL;
    }

    answerSetsCtx *ctx = malloc(sizeof(answerSetsCtx));
    if (!ctx) {
        pclose(fp);
        return NULL;
    }
    ctx->as = NULL;
    ctx->answerSetNum = 0;
    ctx->currentAnswerSetRead = 0;
    ctx->mazeRows = ctx->mazeCols = 0;

    char line[4096];
    int inside_answer = 0;
    AnswerSetModels *currentModel = NULL;
    int parentCount = 0;

    while (fgets(line, sizeof(line), fp))
    {
        //  new answer set
        if (strncmp(line, "Answer:", 7) == 0) 
        {
            if (inside_answer) 
            {
                // finsh  prev model
                if (currentModel && parentCount != currentModel->count) 
                {
                    // trim to the actual number of parents read
                    parent *newModel = realloc(currentModel->data.model,
                                              parentCount * sizeof(parent));

                    if (newModel) currentModel->data.model = newModel;
                    currentModel->count = parentCount;
                }
            }
            // reeset for next answer set
            inside_answer = 1;
            currentModel = NULL;   // will be allocated when first parent is seen
            parentCount = 0;
            continue;
        }

        // Skip unwanted lines
        if (strncmp(line, "clingo version", 14) == 0 ||
            strncmp(line, "Reading from", 12)   == 0 ||
            strncmp(line, "Solving...", 10)     == 0 ||
            strncmp(line, "Optimization:", 13)  == 0 ||
            strncmp(line, "OPTIMUM FOUND", 13)  == 0 ||
            strncmp(line, "Models", 6)          == 0 ||
            strncmp(line, "Calls", 5)           == 0 ||
            strncmp(line, "Time", 4)            == 0 ||
            strncmp(line, "CPU Time", 8)        == 0 ||
            strncmp(line, "SATISFIABLE", 11)    == 0 ||
            strncmp(line, "  Optimum", 9)       == 0)
            continue;

        if (!inside_answer) continue;

        // tokenize line to find parent()
        char *tok = strtok(line, " \t\r\n");
        while (tok) 
        {
            if (strncmp(tok, "parent(", 7) == 0) 
            {
                parent p = getParent(tok);
                if (p.x < 0) continue; // not good

                // must have models  --TODO:REFACTOR THIS 
                if (!currentModel) 
                {
                    AnswerSets *sets = ctx->as;
                    
                    if (!sets) 
                    {
                        sets = malloc(sizeof(AnswerSets));
                        if (!sets) { pclose(fp); free(ctx); return NULL; }
                        sets->count = 0;
                        sets->mods = NULL;
                        sets->f = MAZE_FORMAT_PARENT;
                        ctx->as = sets;
                    }
                    // dynamic growth for the  array
                    int newCount = sets->count + 1;
                    AnswerSetModels *newMods = realloc(sets->mods, 
                                                      newCount * sizeof(AnswerSetModels));
                    if (!newMods) //failure
                    { 
                        pclose(fp); 
                        freeAnswerSets(sets); 
                        free(ctx); 
                        return NULL; 
                    }
                    sets->mods = newMods;
                    sets->mods[newCount-1].count = 0;
                    sets->mods[newCount-1].data.model = NULL;
                    sets->count = newCount;

                    currentModel = &sets->mods[newCount-1];
                }

                // dynamic growth
                if (parentCount >= currentModel->count) 
                {
                    int newCap = currentModel->count == 0 ? 16 : currentModel->count * 2;

                    parent *newModel = realloc(currentModel->data.model, 
                                             newCap * sizeof(parent));
                    if (!newModel) 
                    {
                        pclose(fp); 
                        freeAnswerSets(ctx->as); 
                        free(ctx);
                        return NULL;
                    }
                    currentModel->data.model = newModel;
                    currentModel->count = newCap;
                }
                currentModel->data.model[parentCount++] = p;
            }
            tok = strtok(NULL, " \t\r\n");
        }
    }

    // Close the last model 
    if (inside_answer && currentModel) 
    {
        if (parentCount != currentModel->count) 
        {
            parent *newModel = realloc(currentModel->data.model, 
                                        parentCount * sizeof(parent));
            if (newModel) currentModel->data.model = newModel;
            currentModel->count = parentCount;
        }
    }

    pclose(fp);
    ctx->answerSetNum = ctx->as ? ctx->as->count : 0;
    return ctx;
}
answerSetsCtx* parseClingoEdgeOutput(const char *coreFile, const char *factsFile)
{
    char cmd[65536];
    snprintf(cmd, sizeof(cmd), "clingo %s %s --sign-def=rnd --rand-freq=1 -n 200", coreFile, factsFile);

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen(clingo)");
        return NULL;
    }

    

    answerSetsCtx *ctx = malloc(sizeof(answerSetsCtx));
    if (!ctx) {
        pclose(fp);
        return NULL;
    }

    ctx->as = NULL;
    ctx->answerSetNum = 0;
    ctx->currentAnswerSetRead = 0;
    ctx->mazeRows = 0;
    ctx->mazeCols = 0;

    char line[10000];
    int inside_answer = 0;
    AnswerSetModels *currentModel = NULL;
    int edgeCount = 0;

    while (fgets(line, sizeof(line), fp))
    {
        printf("%s\n",line);
        //new answer set
        if (strncmp(line, "Answer:", 7) == 0) 
        {
            //finish prev answer set
            if (inside_answer && currentModel) 
            {
                if (edgeCount != currentModel->count) 
                { 
                    //trim 
                    edge *newEdges = realloc(currentModel->data.edgeModels,
                                             edgeCount * sizeof(edge));
                    if (newEdges || edgeCount == 0)
                        currentModel->data.edgeModels = newEdges;
                    currentModel->count = edgeCount;
                }
            }

            inside_answer = 1;
            currentModel = NULL;
            edgeCount = 0;
            continue;
        }

        if (strncmp(line, "clingo version", 14) == 0 ||
            strncmp(line, "Reading from", 12)   == 0 ||
            strncmp(line, "Solving...", 10)     == 0 ||
            strncmp(line, "Optimization:", 13)  == 0 ||
            strncmp(line, "OPTIMUM FOUND", 13)  == 0 ||
            strncmp(line, "Models", 6)          == 0 ||
            strncmp(line, "Calls", 5)           == 0 ||
            strncmp(line, "Time", 4)            == 0 ||
            strncmp(line, "CPU Time", 8)        == 0 ||
            strncmp(line, "SATISFIABLE", 11)    == 0 ||
            strncmp(line, "  Optimum", 9)       == 0)
            continue;

        if (!inside_answer)
            continue;

        char *tok = strtok(line, " \t\r\n");
        while (tok) 
        {
            if (strncmp(tok, "edge(", 5) == 0) 
            {
                edge e = getEdge(tok);

                if (e.x < 0 || e.y < 0)
                {
                    //tokenize edge()
                    tok = strtok(NULL, " \t\r\n");
                    continue;
                }

                if (!currentModel) 
                {
                    AnswerSets *sets = ctx->as;
                    if (!sets) 
                    {
                        sets = malloc(sizeof(AnswerSets));
                    
                        if (!sets) 
                        {
                            pclose(fp);
                            free(ctx);
                            return NULL;
                        }
                        sets->count = 0;
                        sets->mods = NULL;
                        sets->f = MAZE_FORMAT_EDGE;
                        ctx->as = sets;
                    }

                    int newCount = sets->count + 1;
                    AnswerSetModels *newMods =
                        realloc(sets->mods, newCount * sizeof(AnswerSetModels));
                    if (!newMods) 
                    {
                        pclose(fp);
                        freeAnswerSets(sets);
                        free(ctx);
                        return NULL;
                    }

                    sets->mods = newMods;
                    sets->mods[newCount - 1].count = 0;
                    sets->mods[newCount - 1].data.edgeModels = NULL;
                    sets->count = newCount;

                    currentModel = &sets->mods[newCount - 1];
                }

                //dynamic growth
                if (edgeCount >= currentModel->count) 
                {
                    int newCap = (currentModel->count == 0) ? 16 : currentModel->count * 2;
                    edge *newEdges =
                        realloc(currentModel->data.edgeModels, newCap * sizeof(edge));
                    if (!newEdges) 
                    {
                        pclose(fp);
                        freeAnswerSets(ctx->as);
                        free(ctx);
                        return NULL;
                    }
                    currentModel->data.edgeModels = newEdges;
                    currentModel->count = newCap;
                }

                currentModel->data.edgeModels[edgeCount++] = e;
            }

            tok = strtok(NULL, " \t\r\n");
        }
    }

    if (inside_answer && currentModel) 
    {
        if (edgeCount != currentModel->count)
        {
            edge *newEdges = realloc(currentModel->data.edgeModels,
                                     edgeCount * sizeof(edge));
            if (newEdges || edgeCount == 0)
                currentModel->data.edgeModels = newEdges;
            currentModel->count = edgeCount;
        }
    }

    pclose(fp);
    ctx->answerSetNum = ctx->as ? ctx->as->count : 0;
    return ctx;
}

//free  answeer set context
void freeAnswerSetCtx(answerSetsCtx *asc)
{
    if (!asc) return;
    freeAnswerSets(asc->as);
    free(asc);
}
