% Maze solver
% Antony Baker
% March 2026

% given list of edges that represent openings edge(1,2) => means a gap between block (1,1) and block (1,2). row1 - 1,2,3,4,5, ... and row2 - 6,7,8,9,... for 5x5 to 25

% get size of maze
maze(1..N*M) :- size(N,M).
finish(M*N) :- end(N,M).

% Make edges bidirectional so we can travel both ways through a gap
adj(X, Y) :- edge(X, Y).
adj(Y, X) :- edge(X, Y).

% Generate all possible paths
{ path(X, Y) } :- adj(X,Y).

% starting position is trivially accessible
accessible(Z*Y) :- start(Z,Y).

% each edge represents a gap and thus every accessible block must be connected by common edge
accessible(K) :- path(X,K), accessible(X).

% solution to maze must be found
:- finish(F), not accessible(F).

% path must be accessible from previous path
:- path(X,Y), not accessible(X).

% each cell must have only one incoming and one outgoing pathway
:- path(X,V1), path(X,V2), V1 != V2.
:- path(V1,Y), path(V2,Y), V1 != V2.

% finish cannot have outgoing path
:- finish(F), path(F,_).

#show path/2.
