import heapq
from collections import defaultdict

INF = 10**18

# Leftist heap, fits these requirements: a min heap, that has bounded degree, into which items can be inserted persistently
class EHeap:
    def __init__(self, rank, key, value, left, right):
        self.rank = rank
        self.key = key
        self.value = value
        self.left = left
        self.right = right

    @staticmethod
    def insert(a, k, v):
        if not a or k < a.key:
            return EHeap(1, k, v, a, None)
        l, r = a.left, EHeap.insert(a.right, k, v)
        if not l or (r and r.rank > l.rank):
            l, r = r, l
        return EHeap(r.rank + 1 if r else 1, a.key, a.value, l, r)

    def __lt__(self, _):
        return False

# Standard Dijkstra algorithm
def dijkstra(g, src, n):
    dist = [INF] * n
    parent = [-1] * n
    dist[src] = 0
    heap = [(0, src)]
    while heap:
        d_u, u = heapq.heappop(heap)
        if d_u != dist[u]:
            continue
        for w, v in g[u]:
            if d_u + w < dist[v]:
                dist[v] = d_u + w
                parent[v] = u
                heapq.heappush(heap, (dist[v], v))
    return dist, parent

# Eppstein's Algorithm
def eppstein_k_shortest_paths(g, src, dst, k, n):
    revg = [[] for _ in range(n)]
    for u in range(n):
        for w, v in g[u]:
            revg[v].append((w, u))

    d, p = dijkstra(revg, dst, n)
    if d[src] == INF:
        return []

    tree = [[] for _ in range(n)]
    for u in range(n):
        if p[u] != -1:
            tree[p[u]].append(u)

    h = [None] * n
    queue = [dst]
    for u in queue:
        seen_parent = False
        for w, v in g[u]:
            if d[v] == INF:
                continue
            cost_diff = w + d[v] - d[u]
            if not seen_parent and v == p[u] and cost_diff == 0:
                seen_parent = True
                continue
            h[u] = EHeap.insert(h[u], cost_diff, v)
        for v in tree[u]:
            h[v] = h[u]
            queue.append(v)

    result = [d[src]]
    if not h[src]:
        return result

    heap = [(d[src] + h[src].key, h[src])]
    while heap and len(result) < k:
        total_cost, node = heapq.heappop(heap)
        result.append(total_cost)
        if h[node.value]:
            heapq.heappush(heap, (total_cost + h[node.value].key, h[node.value]))
        if node.left:
            heapq.heappush(heap, (total_cost + node.left.key - node.key, node.left))
        if node.right:
            heapq.heappush(heap, (total_cost + node.right.key - node.key, node.right))
    return result

def main():
    print("Enter the number of edges:")
    num_edges = int(input())

    node_map = dict()
    reverse_map = dict()
    next_id = 0
    edges_raw = []

    print("Enter each edge in the format 'from to weight' (e.g., A B 3):")
    for _ in range(num_edges):
        u_str, v_str, w = input().split()
        w = int(w)
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

    n = next_id
    g = [[] for _ in range(n)]
    for u, v, w in edges_raw:
        g[u].append((w, v))

    source_str = input("Enter the source node: ").strip()
    target_str = input("Enter the target node: ").strip()
    k = int(input("Enter the number of paths (k): "))

    if source_str not in node_map or target_str not in node_map:
        print(f"No path found from {source_str} to {target_str}.")
        return

    source = node_map[source_str]
    target = node_map[target_str]

    results = eppstein_k_shortest_paths(g, source, target, k, n)

    if not results:
        print(f"No path found from {source_str} to {target_str}.")
    else:
        for i, cost in enumerate(results):
            print(f"Path {i + 1}: {cost}")
        for _ in range(k - len(results)):
            print("-1")

if __name__ == "__main__":
    main()