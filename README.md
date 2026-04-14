# Maze Visualizer

An ASP-based maze generation and visualization project written in C. The program uses **Clingo** to generate mazes from Answer Set Programming encodings, converts the resulting answer sets into an internal maze representation, solves each maze with a second ASP program, and displays everything in an interactive **SDL2** viewer.

## Project overview

This submission demonstrates how ASP can be used not only to model maze generation, but also to support maze solving and inspection. The program:

1. builds a facts file for the requested maze size,
2. runs a maze generator in Clingo,
3. parses the generated answer sets,
4. converts each answer set into a maze grid with walls/passages,
5. solves each maze using a separate ASP solver, and
6. visualizes the generated mazes and their solutions in an SDL2 application.

## Main features

- Maze generation using ASP and Clingo
- Support for two generator modes:
  - **gen0**: edge-based generator
  - **gen1**: parent-based generator
- Parsing of Clingo output into C data structures
- Conversion from answer sets to maze wall layouts
- Maze solving using a separate ASP solver
- Interactive SDL2 visualization of:
  - the current maze,
  - the solution path,
  - parent atoms,
  - generated edge facts, and
  - raw solver output
- Support for browsing multiple generated answer sets/mazes

## Folder layout

In the repository, all project files are inside the `maze-viz/` folder.

Typical structure:

```text
repo-root/
└── maze-viz/
    ├── main.c
    ├── mazeViz.c
    ├── mazeConverts.c
    ├── mazeApp.h
    ├── answerSetParser.c
    ├── answerSetParser.h
    ├── maze-core.lp
    ├── maze_solver.lp
    ├── CMakeLists.txt
    ├── run.sh
    └── ...
```

## File descriptions

- **main.c**  
  Entry point of the program. It handles generator selection, asks for the maze size, runs Clingo, builds mazes from answer sets, solves them, and launches the SDL2 viewer.

- **answerSetParser.h / answerSetParser.c**  
  Defines the core data structures for answer sets and implements parsing of Clingo output for both parent-based and edge-based encodings.

- **mazeConverts.c**  
  Converts maze wall layouts into edge facts for the solver, solves mazes using Clingo, and provides helper utilities for exporting/debugging edge files.

- **mazeViz.c**  
  Implements the SDL2-based graphical visualizer and user interaction.

- **mazeApp.h**  
  Shared declarations for maze solving, exporting, and visualization.

- **maze-core.lp**  
  Parent-based ASP maze generator.

- **maze_solver.lp**  
  ASP maze solver used to compute a valid path from start to finish.

- **CMakeLists.txt**  
  Build configuration for compiling the project with CMake.

- **run.sh**  
  Convenience script that configures the project (if needed), builds it, and runs the executable.

## Requirements

To build and run the project, you need:

- A C compiler with **C11** support
- **CMake 3.16** or newer
- **SDL2** development libraries
- **Clingo** installed and available in your `PATH`

## Building the project

Open a terminal and go to the project folder:

```bash
cd maze-viz
```

### Option 1: Use the provided script

```bash
chmod +x run.sh
./run.sh
```

### Option 2: Build manually with CMake

```bash
cmake -S . -B build
cmake --build build
```

Then run the program:

```bash
./build/maze_visualizer --g gen1
```

or

```bash
./build/maze_visualizer --g gen0
```

If `--g` is omitted, the program defaults to **gen0**.

## How to use

When the program starts, it prompts for the maze dimension:

```text
Enter the length of your maze (default=5):
```

For example, entering `10` creates a `10 x 10` maze.

The program generates answer sets with Clingo, converts them into mazes, solves them, and opens the SDL2 visualizer.

## Viewer controls

### Keyboard

- **Left Arrow** or **A**: previous maze
- **Right Arrow** or **D**: next maze
- **Home**: jump to the first maze
- **End**: jump to the last maze
- **S** or **Enter**: toggle solution path on/off
- **1**: show parent information
- **2**: show generated edge facts
- **3**: show raw solver output

### Mouse

- **Mouse wheel**: scroll the information panel

### On-screen buttons

The interface also includes buttons for:

- **PREV**
- **NEXT**
- **SOLVE**
- **PARENTS**
- **EDGES**
- **SOLVER**

## Program flow

The execution pipeline is:

1. Generate `facts.lp` for the chosen dimension.
2. Run the selected ASP maze generator in Clingo.
3. Parse the resulting answer sets.
4. Convert each answer set into a maze representation using wall bitmasks.
5. Solve each maze with `maze_solver.lp`.
6. Launch the SDL2 viewer to inspect the mazes and solutions.

## Generated/runtime files

During execution, the program may create helper files such as:

- `facts.lp` – size facts for the maze generator
- `edges.lp` – edge facts for solving a maze
- `original_edges.lp` – exported edges from generated answer sets

The viewer can also run the solver on-demand using temporary edge files.

## Notes and limitations

- The project depends on **Clingo** being installed and accessible from the terminal.
- SDL2 must be installed before building.
- The solver view currently displays **raw, unparsed Clingo output**.
- The program is primarily set up for local execution from the project folder so the `.lp` files can be found correctly.

## Educational value

This project is useful for demonstrating:

- Answer Set Programming in a practical application
- Parsing solver output in C
- Translating declarative models into graphical structures
- Solving and visualizing search problems interactively
- Combining ASP with low-level systems programming and SDL2 graphics

## Author

**Augustine Mochoeneng**
**Antony Baker**
**Gideon Weiss**
