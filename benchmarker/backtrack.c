// backtracking algortihmn in C
// Augustine MOCHOENENG
/*
 *having an nXn grid array
 1. start at 0,0
 2. make cell as visited
 3. get unvisited neighbours
 4. if amy unvisted neighbour
    4.1 choose random neighbour
    4.2 make passage from current cell to chosen neighbour
    4.3 go to step 2 (recurse)
5. if no unvisted neighbours
   5.1 backtrack (return to caller)
   5.2 all cells visited?
*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
i64 Width, height;
typedef enum {
  NORTH = 1 << 0,
  EAST = 1 << 1,
  SOUTH = 1 << 2,
  WEST = 1 << 3
} direction;
// directioNS
static const direction ALL_DIRS[4] = {NORTH, EAST, SOUTH, WEST};
static const i8 DX[4] = {0, 1, 0, -1};
static const i8 DY[4] = {-1, 0, 1, 0};
static inline i32 indexOf(i32 x, i32 y) { return y * Width + x; }
static direction opposite(direction d) {
  switch (d) {
    case NORTH:
      return SOUTH;
    case SOUTH:
      return NORTH;
    case EAST:
      return WEST;
    case WEST:
      return EAST;
    default:
      return 0;
  }
}
static void shuffleNeighbour(u8* arr, u8 n) {
  for (u8 i = n - 1; i > 0; i--) {
    u8 j = rand() % (i + 1);

    // swap
    u8 temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
  }
}

// frame , replaces the stack ..only push info we care about
typedef struct {
  i32 x, y;
  u8 order[4];  // keep the shuffled directions, to visit them in order
  u8 step;      // direction index we trying next
} Frame;

static void carve(u32 x, u32 y, direction* grid) {
  // max size:
  u64 capacity = (u64)(Width * height);
  Frame* stack = malloc(capacity * sizeof(Frame));
  i64 top = 0; /* index of the top frame */

  // step 2 , marking start as visitded
  Frame* f = &stack[top];
  f->x = (i32)x;
  f->y = (i32)y;
  f->step = 0;
  f->order[0] = 0;
  f->order[1] = 1;
  f->order[2] = 2;
  f->order[3] = 3;
  shuffleNeighbour(f->order, 4);
  // a visited cell has at least one passage...the first cell has none
  grid[indexOf(x, y)] = 0xFF;

  while (top >= 0) {
    f = &stack[top];

    // try the next unvisited neighbour
    u8 found = 0;
    while (f->step < 4) {
      u8 idx = f->order[f->step];
      f->step++;

      direction d = ALL_DIRS[idx];
      i64 nx = f->x + DX[idx];
      i64 ny = f->y + DY[idx];

      if (nx < 0 || nx >= Width || ny < 0 || ny >= height) continue;
      if (grid[indexOf(nx, ny)] != 0)  // visisted
        continue;

      // make passage
      if (grid[indexOf(f->x, f->y)] == (direction)0xFF)
        grid[indexOf(f->x, f->y)] = 0;

      grid[indexOf(f->x, f->y)] |= d;
      grid[indexOf(nx, ny)] |= opposite(d);

      // 4.3 go to step 2 — push neighbour onto tos
      top++;
      Frame* next = &stack[top];
      next->x = (i32)nx;
      next->y = (i32)ny;
      next->step = 0;
      next->order[0] = 0;
      next->order[1] = 1;
      next->order[2] = 2;
      next->order[3] = 3;
      shuffleNeighbour(next->order, 4);

      found = 1;
      break;
    }

    // 5. no unvisited neighbours ...backtrack
    if (!found) {
      if (grid[indexOf(f->x, f->y)] == (direction)0xFF)
        grid[indexOf(f->x, f->y)] = 0;

      top--;
    }
  }

  free(stack);
}

static void print_maze(direction* grid) {
  // top border
  printf(" ");
  for (i32 x = 0; x < Width; x++) printf(" _");
  printf("\n");
  for (i32 y = 0; y < height; y++) {
    printf("|");
    for (i32 x = 0; x < Width; x++) {
      direction cell = grid[indexOf(x, y)];
      // bottom
      if (cell & SOUTH)
        printf(" ");
      else
        printf("_");
      // right wall
      if (cell & EAST)
        printf(" ");
      else
        printf("|");
    }
    printf("\n");
  }
}

void countFeatures(direction* grid, i64* deadEnds, i64* intersections) {
  *deadEnds = 0;
  *intersections = 0;

  for (i32 y = 0; y < height; y++) {
    for (i32 x = 0; x < Width; x++) {
      direction cell = grid[indexOf(x, y)];
      int degree = 0;

      if (cell & NORTH) degree++;
      if (cell & EAST) degree++;
      if (cell & SOUTH) degree++;
      if (cell & WEST) degree++;

      if (degree == 1) (*deadEnds)++;
      if (degree >= 3) (*intersections)++;
    }
  }
}

int main(i32 argc, char* argv[]) {
  if (argc < 3) {
    return 1;
  }
  Width = atoi(argv[1]);
  height = atoi(argv[2]);
  direction* grid = calloc((u64)(Width * height), sizeof(direction));
  srand((unsigned)time(NULL));
  carve(0, 0, grid);
  i64 deadEnds, intersections;
  countFeatures(grid, &deadEnds, &intersections);
  printf("Dead ends: %lld\nIntersections: %lld\n", deadEnds, intersections);
  print_maze(grid);
  free(grid);
  return 0;
}
