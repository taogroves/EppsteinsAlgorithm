## Puzzle: Flee the Island!
You run a shady criminal organization on a secluded island that’s now under police surveillance. You plan to relocate to a safer island. There is a large network of airports and flights routes, and you have *k* key members who all need one-way flights to a final island with the shortest possible travel times.

However, if two members arrive at the same time, it draws unwanted attention! Therefore, each member must arrive at a different time. Your goal is to figure out the time it will take to transport all members while following the below constraints:
- All members must reach the destination in the shortest possible time
- No two members may arrive at the exact same time
- Paths may contain loops, but members may not wait at any island. They must be on a flight at all times until they arrive at the destination.

### Input Format
First line: n m s t k  
n = number of islands (nodes in the graph)  
m = number of flight routes (edges in the graph)  
s = index of the island you are fleeing from  
t = index of the final destination island  
k = number of organization members  

Next m lines: u v w  
A flight route goes from island u to island v (**not** from v to u) with a travel time w.
### Output Format
Assuming all *k* members begin their journey at time t=0, output the time at which they will all have arrived at the destination. In other words, the length of the longest path found.

### Example
Suppose you have:
```
7 11 0 5 4
0 1 2
0 5 3
0 2 4
1 3 2
2 3 2
3 4 3
4 5 1
3 5 4
2 6 2
6 5 1
6 4 1
```
Answer: **10**

Here:
- There are 7 islands (0 to 6).
- There are 11 flight routes between islands.
- Your starting island is island 0.
- Your final safe island is island 5.
- You are moving 4 members (k=4).
- Hint: you will need to compute at least 6 paths to get the correct answer for this example.