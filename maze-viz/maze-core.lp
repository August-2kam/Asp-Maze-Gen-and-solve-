%Augustine Mochoeneng
%Maze Generator based on the paper : ASP with applications by Mark J. Nelson and Adam M. Smith 
%08-March-2026

start(1,1).
%parent(x,y,dx, dy) -> a path from cell (x,y) -> (x+dx, d+dy) 
%cell chose one parent in one of the 4 dirs
1 { parent(X,Y, 0,-1) ; parent(X,Y, 1,0) ; parent(X,Y,-1,0) ; parent(X,Y,0,1) } 1 :-
    dim(X), dim(Y), (X,Y) != (1,1).



%make sure parent is inside the maze
:- parent(X,Y,DX,DY), PX = X+DX, PY = Y+DY, not dim(PX).
:- parent(X,Y,DX,DY), PX = X+DX, PY = Y+DY, not dim(PY).

%reachabilibity ,spanning tree
reachable(1,1).

reachable(X,Y) :-
    dim(X), dim(Y),
    parent(X,Y,DX,DY),
    PX = X+DX, PY = Y+DY,
    reachable(PX,PY).

:- dim(X), dim(Y), not reachable(X,Y).   %connectivity

%the tree ust have atleast cell with two children, (without this some mazes had weird straight line solutions)
child(CX,CY,PX,PY) :- parent(CX,CY,DX,DY), PX = CX+DX, PY = CY+DY.

%branching node = have atleast two children.
branching_required :- child(_,_, PX,PY), child(_,_, PX,PY), child1 != child2.
:- not branching_required.   %atleats one

% tweaking the number of direction changes 

% Direction of the edge from a child to its parent.
dir(X,Y,DX,DY) :- parent(X,Y,DX,DY).

%bend = ( direction from cell -> parent != direction from parent -> grandparent)
bend(X,Y) :-
    parent(X,Y,DX,DY),
    PX = X+DX, PY = Y+DY,
    dir(PX,PY,DX2,DY2),
    (DX,DY) != (DX2,DY2),
    not start(PX,PY).           %no need to enforce the bend on root


#maximize { 1,X,Y : bend(X,Y) }. % "difficulty" mode


%#minimize { 1,X,Y : bend(X,Y) }. %easy  mode 

#show parent/4.
