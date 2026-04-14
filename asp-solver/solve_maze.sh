clingo maze.lp input.lp --outf=2 --rand-freq=1 | jq -r '.Call[0].Witnesses[0].Value[] + "."' > atoms.lp
clingo maze_solver.lp atoms.lp --outf=2 --rand-freq=1 | jq -r '.Call[0].Witnesses[0].Value[] + "."' > atoms_solved.lp 
cat atoms.lp atoms_solved.lp > maze_and_solution.txt
