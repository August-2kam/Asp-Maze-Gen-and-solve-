# Maze Visualizer

An ASP-based maze generation and visualization project written in C. The program uses **Clingo** to generate mazes from Answer Set Programming encodings, converts the resulting answer sets into an internal maze representation, solves each maze with a second ASP program, and displays everything in an interactive **SDL2** viewer.

## Project overview

This submission demonstrates how ASP can be used to model maze generation, maze solving, and solution inspection. The program:

1. builds a facts file for the requested maze size,
2. runs the maze generator in Clingo,
3. parses the generated answer sets,
4. converts each answer set into a maze grid with walls and passages,
5. solves each maze using a separate ASP solver, and
6. visualizes the generated mazes and their solutions in an SDL2 application.

## Main features

- Maze generation using ASP and Clingo
- Parsing of Clingo output into C data structures
- Conversion from answer sets to maze wall layouts
- Maze solving using a separate ASP solver
- Interactive SDL2 visualization of:
  - the current maze,
  - the solution path,
  - generated edge facts, and
  - raw solver output
- Support for browsing multiple generated answer sets and mazes

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
    ├── CMakeLists.txt
    ├── run.sh
    ├── README.md
    ├── encodings/
    │   ├── maze.lp
    │   ├── maze_solver.lp
    │   └── helper shell script
    └── ...
```

## File descriptions

- **main.c**  
  Entry point of the program. It asks for the maze size, runs Clingo, builds mazes from answer sets, solves them, and launches the SDL2 viewer.

- **answerSetParser.h / answerSetParser.c**  
  Defines the core data structures for answer sets and implements parsing of Clingo output.

- **mazeConverts.c**  
  Converts maze wall layouts into edge facts for the solver, solves mazes using Clingo, and provides helper utilities for exporting or debugging edge files.

- **mazeViz.c**  
  Implements the SDL2-based graphical visualizer and user interaction.

- **mazeApp.h**  
  Shared declarations for maze solving, exporting, and visualization.

- **encodings/**  
  Contains the ASP encodings used by the project, including the maze generator and maze solver, as well as a helper shell script for generating atoms and combining maze and solution output.

- **CMakeLists.txt**  
  Build configuration for compiling the project with CMake.

- **run.sh**  
  Convenience script that configures the project, builds it, and runs the executable.

## Requirements

To build and run the project, you need:

- A C compiler with **C11** support
- **CMake 3.16** or newer
- **SDL2** development libraries
- **Clingo** installed and available in your `PATH`
- `jq` installed if you want to use the helper shell script inside `encodings/`

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
./build/maze_visualizer
```

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
- **S** or **Enter**: toggle solution path on or off
- **2**: show generated edge facts
- **3**: show raw solver output

### Mouse

- **Mouse wheel**: scroll the information panel

### On-screen buttons

The interface also includes buttons for:

- **PREV**
- **NEXT**
- **SOLVE**
- **EDGES**
- **SOLVER**

## Program flow

The execution pipeline is:

1. Generate `facts.lp` for the chosen dimension.
2. Run the ASP maze generator in Clingo.
3. Parse the resulting answer sets.
4. Convert each answer set into a maze representation using wall bitmasks.
5. Solve each maze with the maze solver encoding.
6. Launch the SDL2 viewer to inspect the mazes and solutions.

## `encodings/` helper script

Inside the `encodings/` folder there is also a shell script for generating the maze atoms, solving the maze, and combining both outputs into one text file:

```bash
#!/bin/bash

clingo maze.lp facts.lp --outf=2 --sign-def=rnd --rand-freq=1 | jq -r '.Call[0].Witnesses[0].Value[] + "."' > atoms.lp
clingo maze_solver.lp  atoms.lp --outf=2 --rand-freq=1 | jq -r '.Call[0].Witnesses[0].Value[] + "."' > atoms_solved.lp 
cat atoms.lp atoms_solved.lp > maze_and_solution.txt
```

This script is useful for producing a plain-text version of the generated maze atoms together with the solver atoms.

## Generated/runtime files

During execution, the program may create helper files such as:

- `facts.lp` - size facts for the maze generator
- `edges.lp` - edge facts for solving a maze
- `atoms.lp` - generated maze atoms from Clingo
- `atoms_solved.lp` - solver atoms for the maze solution
- `maze_and_solution.txt` - combined maze and solution output

The viewer can also run the solver on demand using temporary edge files.

## Notes and limitations

- The project depends on **Clingo** being installed and accessible from the terminal.
- SDL2 must be installed before building.
- The solver view currently displays **raw, unparsed Clingo output**.
- The maze dimension should be **less than or equal to 50**.
- For large mazes, maze solving can take noticeably longer, so the user may need to wait a while for solutions to be produced.
- The program is primarily set up for local execution from the project folder so the `.lp` files can be found correctly.

## About the answer sets

This project visualizes the **answer sets produced by Clingo**.

Some generated mazes may look similar, and some solutions may also look similar. This is expected: in ASP, multiple answer sets can satisfy the same declarative constraints in structurally similar ways. Because of that, different valid mazes produced by the encoding can still share many common patterns.

## Educational value

This project is useful for demonstrating:

- Answer Set Programming in a practical application
- Parsing solver output in C
- Translating declarative models into graphical structures
- Solving and visualizing search problems interactively
- Combining ASP with low-level systems programming and SDL2 graphics

## Author

**Augustine Mochoeneng**
**Antony Baker**.
**Gideon Weiss**
