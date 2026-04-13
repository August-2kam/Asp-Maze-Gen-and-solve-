#include "mazeApp.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define make_dir(path) _mkdir(path)
#else
#define make_dir(path) mkdir(path, 0755)
#endif

static inline int cellId1Based(int cols, int r, int c)
{
    return r * cols + c + 1;
}

//free the path model
static void 
freePathModel(PathModel *m)
{
    if (!m) return;
    free(m->edges);
    m->edges = NULL;
    m->count = 0;
    m->cap = 0;
}


//a dynami9c array behaviour functiont to append to the array, 
//re-alloc if needed
static int 
pushPathEdge(PathModel *m, int from, int to)
{
    if (!m) return 0;

    if (m->count == m->cap) 
    {
        int newCap = (m->cap == 0) ? 16 : m->cap * 2;
        PathEdge *tmp = realloc(m->edges, (size_t)newCap * sizeof(PathEdge));
        if (!tmp) return 0;
        m->edges = tmp;
        m->cap = newCap;
    }


    m->edges[m->count].from = from;
    m->edges[m->count].to = to;
    m->count++;
    return 1;
}


//write edges to file
int 
writeMazeEdgesToFile(const char *filename, 
                     const uint8_t *cellWalls, 
                     int rows, 
                     int cols)
{
    if (!filename || !cellWalls || rows <= 0 || cols <= 0)
        return 0;

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen(edge file)");
        return 0;
    }

    fprintf(fp, "start(1,1).\n");
    fprintf(fp, "end(%d,%d).\n", rows, cols);
    fprintf(fp, "size(%d,%d).\n", rows, cols);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            int from = cellId1Based(cols, r, c);

            if (c + 1 < cols && !(cellWalls[idx] & EAST)) {
                int to = cellId1Based(cols, r, c + 1);
                fprintf(fp, "edge(%d,%d).\n", from, to);
            }
            if (r + 1 < rows && !(cellWalls[idx] & SOUTH)) {
                int to = cellId1Based(cols, r + 1, c);
                fprintf(fp, "edge(%d,%d).\n", from, to);
            }
        }
    }

    fclose(fp);
    return 1;
}

int 
solveMaze(const uint8_t *maze, int rows, 
                               int cols,
                               const char *solverFile, 
                               PathModel *out)
{
    if (!maze || !solverFile || !out) return 0;
    freePathModel(out);

    if (!writeMazeEdgesToFile("edges.lp", maze, rows, cols))
        return 0;


    //run the solver
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "clingo %s edges.lp -n 1", solverFile);
    FILE *paths = popen(cmd, "r");
    if (!paths) {
        perror("popen(clingo)");
        return 0;
    }

    //parse the input
    char buffer[1024*2];
    while (fgets(buffer, sizeof(buffer), paths)) 
    {
        char *tok = strtok(buffer, " \t\r\n");
        while (tok) 
        {
            int a, b;
            if (sscanf(tok, "path(%d,%d)", &a, &b) == 2) 
            {
                if (!pushPathEdge(out, a, b)) {
                    pclose(paths);
                    freePathModel(out);
                    return 0;
                }
            }
            tok = strtok(NULL, " \t\r\n");
        }
    }

    pclose(paths);
    return 1;
}


//solve all the mazes in one go.
PathSet* 
solveAllMazes(uint8_t **mazes, int mazeCount, 
                              int rows, 
                              int cols, 
                              const char *solverFile)
{
    if (!mazes || mazeCount <= 0 || rows <= 0 || cols <= 0 || !solverFile)
        return NULL;

    PathSet *paths = malloc(sizeof(PathSet));
    if (!paths) return NULL;

    paths->count = mazeCount;
    paths->mods = calloc((size_t)mazeCount, sizeof(PathModel));
    if (!paths->mods) {
        free(paths);
        return NULL;
    }

    for (int i = 0; i < mazeCount; i++) {
        if (!solveMaze(mazes[i], rows, cols, solverFile, &paths->mods[i])) {
            //failed solves
            freePathModel(&paths->mods[i]);
        }
    }

    return paths;
}

void 
freePathSet(PathSet *paths)
{
    if (!paths) return;
    for (int i = 0; i < paths->count; i++)
        freePathModel(&paths->mods[i]);
    free(paths->mods);
    free(paths);
}

static int ensureDirectoryExists(const char *dirName)
{
    struct stat st;
    if (!dirName) return 0;

    if (stat(dirName, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return 1;
        fprintf(stderr, "Error: '%s' exists but is not a directory\n", dirName);
        return 0;
    }

    if (make_dir(dirName) != 0) {
        perror("mkdir");
        return 0;
    }
    return 1;
}



//TODO: OR NOT TO DO - //
//basically function save all temp file for debugging - might 
//have to show the files on the visualizer itsekf
int dumpAllMazeEdgeFiles(const char *dirName,
                         uint8_t **mazes,
                         int mazeCount,
                         int rows,
                         int cols)
{
    if (!dirName || !mazes || mazeCount <= 0 || rows <= 0 || cols <= 0)
        return 0;
    if (!ensureDirectoryExists(dirName)) return 0;

    for (int i = 0; i < mazeCount; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/maze_%03d.lp", dirName, i + 1);
        if (!writeMazeEdgesToFile(path, mazes[i], rows, cols)) {
            fprintf(stderr, "Error: failed to write %s\n", path);
            return 0;
        }
    }
    return 1;
}

char* generateEdgesString(const uint8_t *cellWalls, int rows, int cols)
{
    if (!cellWalls || rows <= 0 || cols <= 0) return NULL;

    // Estimate required buffer size
    size_t maxEdges = (size_t)rows * cols * 2;   // at most two edges per cell
    size_t bufSize = 256 + maxEdges * 64;        // generous
    char *buf = malloc(bufSize);
    if (!buf) return NULL;

    int pos = snprintf(buf, bufSize,
                       "start(1,1).\n"
                       "end(%d,%d).\n"
                       "size(%d,%d).\n",
                       rows, cols, rows, cols);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            int from = r * cols + c + 1;  // 1‑based index for solver

            if (c + 1 < cols && !(cellWalls[idx] & EAST)) {
                int to = r * cols + (c + 1) + 1;
                pos += snprintf(buf + pos, bufSize - pos,
                                "edge(%d,%d).\n", from, to);
            }
            if (r + 1 < rows && !(cellWalls[idx] & SOUTH)) {
                int to = (r + 1) * cols + c + 1;
                pos += snprintf(buf + pos, bufSize - pos,
                                "edge(%d,%d).\n", from, to);
            }
        }
    }

    return buf;
}

char* runSolverOnMaze(const uint8_t *maze, int rows, int cols, const char *solverFile)
{
    if (!maze || rows <= 0 || cols <= 0 || !solverFile) return NULL;

    // Write edges to a temporary file
    char edgesFile[] = "/tmp/edges_XXXXXX";
    int fd = mkstemp(edgesFile);
    if (fd == -1) return NULL;
    close(fd);

    if (!writeMazeEdgesToFile(edgesFile, maze, rows, cols)) {
        unlink(edgesFile);
        return NULL;
    }

    // Build command
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "clingo %s %s", solverFile, edgesFile);

    // Run and capture output
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        unlink(edgesFile);
        return NULL;
    }

    // Read all output into a buffer
    size_t cap = 4096;
    size_t len = 0;
    char *out = malloc(cap);
    if (!out) {
        pclose(fp);
        unlink(edgesFile);
        return NULL;
    }
    out[0] = '\0';

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        size_t line_len = strlen(line);
        if (len + line_len + 1 > cap) {
            cap *= 2;
            char *tmp = realloc(out, cap);
            if (!tmp) {
                free(out);
                pclose(fp);
                unlink(edgesFile);
                return NULL;
            }
            out = tmp;
        }
        memcpy(out + len, line, line_len);
        len += line_len;
        out[len] = '\0';
    }

    pclose(fp);
    unlink(edgesFile);
    return out;
}
