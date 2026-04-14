// Prim algorithm for Maze Generation in C
// March 2026
// Antony Baker

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
  int path;
  int bottom;
  int right;
} Cell;

typedef struct {
  int r1, c1;
  int r2, c2;
} Wall;

int WIDTH, HEIGHT;
Cell** maze;
Wall* walls;

void addWall(int r1, int c1, int r2, int c2, Wall** walls, int* count) {
  (*walls)[*count].r1 = r1;
  (*walls)[*count].c1 = c1;
  (*walls)[*count].r2 = r2;
  (*walls)[*count].c2 = c2;
  (*count)++;
}

void printMaze() {
  // printf("hello");
  for (int j = 0; j < WIDTH; j++) {
    if (j == 0)
      printf("  ");
    else
      printf(" _");
  }
  printf("\n");
  // Print rows
  for (int i = 0; i < HEIGHT; i++) {
    printf("|");  // Leftmost boundary
    for (int j = 0; j < WIDTH; j++) {
      // Print bottom wall (or floor)
      printf("%c", maze[i][j].bottom ? '_' : ' ');

      // Print right wall, or maintain floor continuity if no right wall exists
      if (maze[i][j].right) {
        printf("|");
      } else {
        printf("%c",
               (maze[i][j].bottom || (j < WIDTH - 1 && maze[i][j + 1].bottom))
                   ? '_'
                   : ' ');
      }
    }
    printf("\n");
  }
}

void countFeatures(Cell** maze, int* deadEnds, int* intersections) {
  *deadEnds = 0;
  *intersections = 0;

  for (int y = 0; y < HEIGHT; ++y) {
    for (int x = 0; x < WIDTH; ++x) {
      int degree = 0;

      if (x < WIDTH - 1 && maze[y][x].right == 0) degree++;    // right
      if (y < HEIGHT - 1 && maze[y][x].bottom == 0) degree++;  // down
      if (x > 0 && maze[y][x - 1].right == 0) degree++;        // left
      if (y > 0 && maze[y - 1][x].bottom == 0) degree++;       // up

      if (degree == 1) (*deadEnds)++;
      if (degree >= 3) (*intersections)++;
    }
  }
}

int main(int argc, char* argv[]) {
  WIDTH = atoi(argv[1]);
  HEIGHT = atoi(argv[2]);
  // printf("hellopw");
  srand(time(NULL));
  maze = malloc(HEIGHT * sizeof(Cell*));

  for (int i = 0; i < HEIGHT; i++) {
    maze[i] = malloc(WIDTH * sizeof(Cell));
    for (int j = 0; j < WIDTH; j++) {
      maze[i][j].path = 0;
      maze[i][j].bottom = 1;
      maze[i][j].right = 1;
    }
  }

  int count = 0;
  walls = malloc(WIDTH * HEIGHT * 2 *
                 sizeof(Wall));  // each cell will have two walls

  // printf("%d", !walls);
  if (!walls) return EXIT_FAILURE;
  // printf("hello");
  //  select random cell from maze
  int row = rand() % HEIGHT;
  int col = rand() % WIDTH;
  maze[row][col].path = 1;

  // add first cell in maze
  if (row > 0) addWall(row, col, row - 1, col, &walls, &count);
  if (row < HEIGHT - 1) addWall(row, col, row + 1, col, &walls, &count);
  if (col > 0) addWall(row, col - 1, row, col, &walls, &count);
  if (col < WIDTH - 1) addWall(row, col + 1, row, col, &walls, &count);

  // main algorithm
  while (count > 0) {
    // select random wall to start
    int idx = rand() % count;
    Wall w = walls[idx];

    walls[idx] = walls[count - 1];  // remove from list
    count--;

    // check if wall is a connection between visisted maze and frontier
    int p1 = maze[w.r1][w.c1].path;
    int p2 = maze[w.r2][w.c2].path;

    // we need only if one is in and the other is out. but not both in or out.
    // exclusive or
    if (p1 ^ p2) {
      int r = p1 ? w.r2 : w.r1;
      int c = p1 ? w.c2 : w.c1;

      // remove wall between cells
      if (w.r1 == w.r2) {
        int left_c = (w.c1 < w.c2) ? w.c1 : w.c2;
        maze[w.r1][left_c].right = 0;
      } else {
        int top_r = (w.r1 < w.r2) ? w.r1 : w.r2;
        maze[top_r][w.c1].bottom = 0;
      }

      maze[r][c].path = 1;

      if (r > 0 && !maze[r - 1][c].path)
        addWall(r, c, r - 1, c, &walls, &count);
      if (r < HEIGHT - 1 && !maze[r + 1][c].path)
        addWall(r, c, r + 1, c, &walls, &count);
      if (c > 0 && !maze[r][c - 1].path)
        addWall(r, c, r, c - 1, &walls, &count);
      if (c < WIDTH - 1 && !maze[r][c + 1].path)
        addWall(r, c, r, c + 1, &walls, &count);
    }
  }

  free(walls);

  int deadEnds, intersections;
  countFeatures(maze, &deadEnds, &intersections);
  printf("Dead ends: %d\nIntersections: %d\n", deadEnds, intersections);

  printMaze();

  for (int i = 0; i < HEIGHT; i++) free(maze[i]);
  free(maze);
}
