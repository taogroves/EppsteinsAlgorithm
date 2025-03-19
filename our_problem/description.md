## Puzzle: Flee the Island!
You run a shady criminal organization on a secluded island that’s now under police surveillance. You plan to relocate to a safer island. There is a large network of airports and flights routes, and you have *k* key members who all need one-way flights to a final island with the shortest possible travel times.

However, if two members arrive at the same time, it draws unwanted attention! Therefore, each member must arrive at a different time. Your goal is to figure out the time it will take to transport all members while following the below constraints:
- All members must reach the destination in the shortest possible time
- No two members may arrive at the exact same time

Input Format
First line: n m s t k
n = number of islands (nodes in the graph)
m = number of flight routes (edges in the graph)
s = index of the island you are fleeing from
t = index of the final destination island
k = number of organization members
Next m lines: u v w
A flight route goes from island u to island v with a travel time w.
Required Output
You must determine the maximum arrival time among the k shortest paths the members can take without sharing the same arrival time at the destination.
Example
Suppose you have:
```
5 6 0 4 3
0 1 2
1 2 2
1 3 4
2 4 6
3 2 1
3 4 3
```

Answer: 12
**More info**

Here:
- There are 5 islands (0 to 4).
- You have 6 flight routes.
- Your starting island is 0.
- Your final safe island is at island 4.
- You are moving 3 members (k=3).