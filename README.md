# balance-strategy-k-server-problem
balance strategy for a k-server problem

compiles with command line  g++ test.c -lX11 -lm

Project description: You have a city map, which is subgraph of a grid graph (some streets are blocked). You have four servers (e.g., repairmen), which are requested at a sequence of grid points; and you dispatch for each request one server. The server stays there until you move him to his next assignment. Your goal is to keep the total travel distance small. Since this is an on-line problem, where the next request comes in only after we served the current request, we cannot expect to get the optimum for the request sequence. Instead we need to implement a local strategy that makes the decision which server to send next. In this project we will use the balance strategy: we choose the server such that the difference in travel distance between the server who traveled least and the server who traveled most is minimized.

More description of the project is included on file "Project3.pdf".
