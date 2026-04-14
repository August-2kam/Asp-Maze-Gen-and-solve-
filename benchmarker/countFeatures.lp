% count the number of deadends and intersections

% undirected neighbor relation from the stored edge facts
neighbor(N, P) :- edge(N, P).
neighbor(N, P) :- edge(P, N).

% count edges per node (degree)
degree(N, D) :- node(N,_,_), D = #count { P : neighbor(N,P) }.

% deadend: node with exactly 1 neighbor
deadend(N) :- degree(N, 1).

% intersection: node with 3 or more neighbors
intersection(N) :- degree(N, D), D >= 3.

% aggregate counts
num_deadends(C) :- C = #count { N : deadend(N) }.
num_intersections(C) :- C = #count { N : degree(N, D), D >= 3 }.

#show num_deadends/1.
#show num_intersections/1.