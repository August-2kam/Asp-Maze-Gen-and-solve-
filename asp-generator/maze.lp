% Maze generator
% Gideon Weiss
% March 2026

#const rows = 40.
#const cols = 40.

size(rows, cols).

start(1,1).
end(rows,cols).

node(N,X,Y) :- N = (X-1)*cols + Y, X=1..rows, Y=1..cols.

% adjacency
adj(N1,N2) :- node(N1,X,Y), node(N2,X,Y+1). % adj down.
adj(N1,N2) :- node(N1,X,Y), node(N2,X+1,Y). % adj right.
adj(N2,N1) :- adj(N1,N2). % undirected

% to create spanning tree, every node except the start node, must have a parent
{ parent(N,P) : adj(N,P) }=1 :- node(N,X,Y), not start(X,Y).

% reachability from start
reachable(N) :- start(X,Y), node(N,X,Y).
reachable(N) :- reachable(P), parent(N,P).
:- node(N,_,_), not reachable(N).

% make the graph undirected. for instance one can only be true. 
% edge(5,6) :- parent(5,6), 5 < 6 which is true. 
% however edge(6,5) :- parent(5,6), 6 < 5 is false. 
% So we eliminate a duplicate
edge(N,P) :- parent(N,P), N < P.
edge(P,N) :- parent(N,P), P < N.

% start and end cannot be the same
:- start(X,Y), end(X,Y).
% #show size/2.
% #show start/2.
% #show end/2.
% #show edge/2.
