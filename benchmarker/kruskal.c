// Randomised Kruskal algorithm for maze generation
// Gideon Weiss
// March 2026

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int WIDTH, HEIGHT;

// create array for each cell, `parent` stores the index of the tree that holds
// cell i
int* parent;

// each wall must be between 2 cells and has a direction
typedef struct {
  int cell1;
  int cell2;
  int direction;  // 0 is right and 1 is down
} Wall;

// we store all possible walls
Wall* walls;
int wallCount = 0;

// flattened 3D array (size HEIGHT * WIDTH * 2)
int* maze;

// macro for 3D access maze[y][x][d]
#define MAZE(y, x, d) maze[(y) * WIDTH * 2 + (x) * 2 + (d)]

// find root of tree
int find(int x) {
  if (parent[x] != x) {
    parent[x] = find(parent[x]);
  }
  return parent[x];
}

// connect 2 sets (ie to connect maze cells)
void unite(int a, int b) { parent[find(a)] = find(b); }

void init() {
  wallCount = 0;

  for (int i = 0; i < WIDTH * HEIGHT; i++)
    parent[i] = i;  // each cell has its own tree to start

  for (int y = 0; y < HEIGHT; ++y) {
    for (int x = 0; x < WIDTH; ++x) {
      // all walls exist initially
      MAZE(y, x, 0) = 1;
      MAZE(y, x, 1) = 1;

      // convert 2d to 1d index
      int id = y * WIDTH + x;

      // if there is a cell to the right, create a wall between them
      if (x < WIDTH - 1) {
        walls[wallCount++] = (Wall){id, id + 1, 0};
      }
      // if there is a cell below, create a wall between them
      if (y < HEIGHT - 1) {
        walls[wallCount++] = (Wall){id, id + WIDTH, 1};
      }
    }
  }
}

// randomise the wall order for kruskals algorithm
void shuffle() {
  for (int i = 0; i < wallCount; ++i) {
    int j = rand() % wallCount;
    Wall temp = walls[i];
    walls[i] = walls[j];
    walls[j] = temp;
  }
}

// core algorithm
void generateMaze() {
  shuffle();

  // loop through all walls
  for (int i = 0; i < wallCount; i++) {
    int a = walls[i].cell1;
    int b = walls[i].cell2;

    // if the cells are connected and different trees, then connect (wont create
    // loop)
    if (find(a) != find(b)) {
      unite(a, b);  // connect cells

      // we need to convert back to 2d representation to remove wall
      int x = a % WIDTH;
      int y = a / WIDTH;

      if (walls[i].direction == 0) {
        MAZE(y, x, 0) = 0;  // remove right wall
      } else {
        MAZE(y, x, 1) = 0;  // remove bottom wall
      }
    }
  }
}

void printMaze() {
  // top border
  for (int x = 0; x < WIDTH; x++) printf(" _");
  printf("\n");

  for (int y = 0; y < HEIGHT; y++) {
    printf("|");
    for (int x = 0; x < WIDTH; x++) {
      printf(MAZE(y, x, 1) ? "_" : " ");
      printf(MAZE(y, x, 0) ? "|" : " ");
    }
    printf("\n");
  }
}

void countFeatures(int* deadEnds, int* intersections) {
  *deadEnds = 0;
  *intersections = 0;

  for (int y = 0; y < HEIGHT; ++y) {
    for (int x = 0; x < WIDTH; ++x) {
      int degree = 0;

      if (x < WIDTH - 1 && MAZE(y, x, 0) == 0) degree++;   // right
      if (y < HEIGHT - 1 && MAZE(y, x, 1) == 0) degree++;  // down
      if (x > 0 && MAZE(y, x - 1, 0) == 0) degree++;       // left
      if (y > 0 && MAZE(y - 1, x, 1) == 0) degree++;       // up

      if (degree == 1) (*deadEnds)++;
      if (degree >= 3) (*intersections)++;
    }
  }
}

int main(int argc, char* argv[]) {
  WIDTH = atoi(argv[1]);
  HEIGHT = atoi(argv[2]);

  srand(time(NULL));

  parent = malloc(WIDTH * HEIGHT * sizeof(int));
  walls = malloc(WIDTH * HEIGHT * 2 * sizeof(Wall));
  maze = malloc(WIDTH * HEIGHT * 2 * sizeof(int));

  init();
  generateMaze();
  int deadEnds, intersections;
  countFeatures(&deadEnds, &intersections);
  printf("Dead ends: %d\nIntersections: %d\n", deadEnds, intersections);

  printMaze();

  free(parent);
  free(walls);
  free(maze);

  return 0;
}