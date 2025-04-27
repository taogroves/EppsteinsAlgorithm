## Puzzle: Flee the Island!
You run a shady criminal organization on a secluded island that’s now under police surveillance. You plan to relocate to a safer island. There is a large network of airports and flights routes, and you have many important members who all need one-way flights to a final island with the shortest possible travel times. However, if two members arrive at the same time, it draws unwanted attention! Therefore, each member must arrive at a different time.

Your goal is to figure out the flight route the *k*-th member must take to reach the new island while following the below constraints:
- The *k*-th member must take the shortest possible journey to the new island.
- The *k*-th member must arrive at a different time than any other member.
- If two routes have the same arrival time, use the one with the shortest average flight duration.
- These constraints must hold for any value of *k* as long as it is possible to find *k* routes with distinct lengths.

In other words, if there are *k* members traveling, each one taking the next shortest route, your algorithm will compute the route the *last* one takes (the *k*-th shortest).

### Input Format
First line: n m s t k  
n = number of islands (nodes in the graph)  
m = number of flight routes (edges in the graph)  
s = index of the island you are fleeing from  
t = index of the final destination island  
k = index of the path to compute

Next m lines: u v w  
A flight route goes from island u to island v (**not** from v to u) with a travel time w.
### Output Format
Output the nodes visited by the *k*-th member on their journey from *s* to *t*, separated by spaces.

### Example
Suppose you have:
```
7 11 0 5 3
0 1 2
0 5 3
0 2 4
1 3 2
2 3 2
3 4 3
4 5 1
3 5 4
2 6 2
6 5 3
6 4 1
```
Answer: **0 2 6 5**

Here:
- There are 7 islands (0 to 6).
- There are 11 flight routes between islands.
- The starting island is island 0.
- The final safe island is island 5.
- You are finding the route for the 3rd member (k=3).
- Hint: you will need to compute at least 5 paths to get the correct answer for this example.
    - The optimal path is (0, 5), with length 3. There are three paths of length 8, only one of which is taken. The path returned has length 9.