import heapq
from collections import defaultdict

INF = float('inf')  # Represents "infinite" distance

class EHeap:
    """
    Leftist heap node for sidetrack edges in Eppstein's algorithm.
    Provides a bounded-degree min-heap supporting persistent insert.
    """
    def __init__(self, rank, key, value, left, right):
        self.rank = rank      # null-path length for leftist property
        self.key = key        # cost difference of this sidetrack
        self.value = value    # target vertex of this sidetrack edge
        self.left = left      # left child heap
        self.right = right    # right child heap

    @staticmethod
    def insert(a, k, v):
        """
        Insert a (key=k, value=v) into heap 'a', returning new root.
        This is persistent: original 'a' remains unchanged.
        """
        # Base case: empty heap or new key is smaller than root
        if not a or k < a.key:
            return EHeap(1, k, v, a, None)
        # Recursively insert into right subtree
        l, r = a.left, EHeap.insert(a.right, k, v)
        # Ensure leftist property: left.rank >= right.rank
        if not l or (r and r.rank > l.rank):
            l, r = r, l
        # Update rank: 1 + rank of right child
        new_rank = (r.rank + 1) if r else 1
        # Return new heap node preserving original key/value
        return EHeap(new_rank, a.key, a.value, l, r)

    def __lt__(self, other):
        # Needed for heapq but actual comparison is by external key
        return False


def dijkstra(g, src, n):
    """
    Standard Dijkstra's algorithm.
    g: adjacency list where g[u] contains (weight, v) pairs
    src: source index
    n: number of vertices
    Returns (dist, parent) arrays.
    """
    dist = [INF] * n
    parent = [-1] * n
    dist[src] = 0
    heap = [(0, src)]  # (distance, vertex)

    while heap:
        d_u, u = heapq.heappop(heap)
        if d_u != dist[u]:
            # skip stale entries
            continue
        # Relax all outgoing edges
        for w, v in g[u]:
            if d_u + w < dist[v]:
                dist[v] = d_u + w
                parent[v] = u
                heapq.heappush(heap, (dist[v], v))
    return dist, parent


def eppstein_k_shortest_paths(g, src, dst, k, n):
    """
    Compute k shortest path costs from src to dst using Eppstein's algorithm.
    g: forward adjacency list
    src, dst: source and destination indices
    k: number of paths to return
    n: number of vertices
    Returns list of up to k path costs in non-decreasing order.
    """
    # Build reverse graph for backward Dijkstra
    revg = [[] for _ in range(n)]
    for u in range(n):
        for w, v in g[u]:
            revg[v].append((w, u))

    # Run Dijkstra from dst on reverse graph
    d, p = dijkstra(revg, dst, n)
    if d[src] == INF:
        # No route exists
        return []

    # Build shortest-path tree from parents
    tree = [[] for _ in range(n)]
    for u in range(n):
        if p[u] != -1:
            tree[p[u]].append(u)

    # h[u] will point to a heap of sidetrack edges for u
    h = [None] * n
    queue = [dst]
    for u in queue:
        seen_parent = False
        # Explore all outgoing edges of u in forward graph
        for w, v in g[u]:
            if d[v] == INF:
                continue  # unreachable branch
            cost_diff = w + d[v] - d[u]
            # Skip the main tree edge once
            if not seen_parent and v == p[u] and cost_diff == 0:
                seen_parent = True
            else:
                # Insert this sidetrack into u's heap
                h[u] = EHeap.insert(h[u], cost_diff, v)
        # Propagate heap pointer to children in shortest-path tree
        for v in tree[u]:
            h[v] = h[u]
            queue.append(v)

    # The very shortest path cost
    result = [d[src]]
    # If no sidetracks available at src, only one path exists
    if not h[src]:
        return result

    # Min-heap of (total_cost, heap_node)
    pq = [(d[src] + h[src].key, h[src])]
    # Extract up to k paths
    while pq and len(result) < k:
        total_cost, node = heapq.heappop(pq)
        result.append(total_cost)
        # Follow the sidetrack chain: main branch first
        if h[node.value]:
            heapq.heappush(pq, (total_cost + h[node.value].key, h[node.value]))
        # Then the siblings in the heap
        if node.left:
            heapq.heappush(pq, (total_cost + node.left.key - node.key, node.left))
        if node.right:
            heapq.heappush(pq, (total_cost + node.right.key - node.key, node.right))

    return result


def main():
    # Read input edges
    num_edges = int(input("Enter the number of edges:\n"))
    node_map = {}
    reverse_map = {}
    next_id = 0
    edges_raw = []

    print("Enter each edge in the format 'from to weight' (e.g., A B 3):")
    for _ in range(num_edges):
        u_str, v_str, w = input().split()
        w = int(w)
        # Assign unique numeric IDs to node labels
        if u_str not in node_map:
            node_map[u_str] = next_id
            reverse_map[next_id] = u_str
            next_id += 1
        if v_str not in node_map:
            node_map[v_str] = next_id
            reverse_map[next_id] = v_str
            next_id += 1
        u, v = node_map[u_str], node_map[v_str]
        edges_raw.append((u, v, w))

    # Build forward adjacency list
    n = next_id
    g = [[] for _ in range(n)]
    for u, v, w in edges_raw:
        g[u].append((w, v))

    # Read query
    source_str = input("Enter the source node: ").strip()
    target_str = input("Enter the target node: ").strip()
    k = int(input("Enter the number of paths (k): "))

    # Validate existence
    if source_str not in node_map or target_str not in node_map:
        print(f"No path found from {source_str} to {target_str}.")
        return

    src = node_map[source_str]
    dst = node_map[target_str]
    results = eppstein_k_shortest_paths(g, src, dst, k, n)

    # Output results, pad missing with -1
    if not results:
        print(f"No path found from {source_str} to {target_str}.")
    else:
        for i, cost in enumerate(results, 1):
            print(f"Path {i}: {cost}")
        for _ in range(k - len(results)):
            print("-1")

if __name__ == "__main__":
    main()
