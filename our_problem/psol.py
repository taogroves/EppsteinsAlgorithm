from heapq import heappop, heappush

INF = 10**18

def main():
    n, m, s, t, k = map(int, input().split())
    g = [[] for _ in range(n)]
    for _ in range(m):
        u, v, w = map(int, input().split())
        g[u].append((w, v))

    print(sum(l[0] for l in shortest_paths_no_same_arrival(g, s, t, k)))

def dijkstra(g, src):
    n = len(g)
    d, p = [INF] * n, [-1] * n
    d[src] = 0
    q = [(0, src)]
    paths = []
    while q:
        du, u = heappop(q)
        if du != d[u]:
            continue
        for w, v in g[u]:
            if du + w < d[v]:
                d[v] = du + w
                p[v] = u
                heappush(q, (d[v], v))
    return d, p

# Leftist heap
class EHeap:
    def __init__(self, rank, key, origin, value, left, right):
        self.rank = rank
        self.key = key
        self.origin = origin
        self.value = value
        self.left = left
        self.right = right

    @staticmethod
    def insert(a, k, u, v):
        if not a or k < a.key:
            return EHeap(1, k, u, v, a, None)
        l, r = a.left, EHeap.insert(a.right, k, u, v)
        if not l or r.rank > l.rank:
            l, r = r, l
        return EHeap(r.rank + 1 if r else 1, a.key, a.origin, a.value, l, r)

    def __lt__(self, _):
        return False

    def __repr__(self):
        res = []
        q = [self]
        while q:
            a = q.pop()
            res.append((a.key, a.value))
            if a.left:
                q.append(a.left)
            if a.right:
                q.append(a.right)
        return str(res)

def shortest_paths_no_same_arrival(g, src, dst, k):
    n = len(g)
    revg = [[] for _ in range(n)]
    for u in range(n):
        for w, v in g[u]:
            revg[v].append((w, u))
    d, p = dijkstra(revg, dst)
    if d[src] == INF:
        return []

    t = [[] for _ in range(n)]
    for u in range(n):
        if p[u] != -1:
            t[p[u]].append(u)

    h = [None] * n
    q = [dst]
    for u in q:
        seenp = False
        for w, v in g[u]:
            if d[v] == INF:
                continue
            c = w + d[v] - d[u]
            if not seenp and v == p[u] and c == 0:
                seenp = True
                continue
            h[u] = EHeap.insert(h[u], c, u, v)
        for v in t[u]:
            h[v] = h[u]
            q.append(v)


    ans = [(d[src], [])]

    if not h[src]:
        return ans

    q = [(d[src] + h[src].key, h[src], [])]
    while q and len(ans) < k:
        cd, ch, path = heappop(q)
        # print('key', ch.key)
        new_path = path + [(ch.origin, ch.value)]

        # if the length of this path is the same as the last one, skip it
        if cd > ans[-1][0]:
            ans.append((cd, new_path))

        if h[ch.value]:
            heappush(q, (cd + h[ch.value].key, h[ch.value], new_path))
        if ch.left:
            # print('hi')
            heappush(q, (cd + ch.left.key - ch.key, ch.left, path))
        if ch.right:
            # print('bye')
            heappush(q, (cd + ch.right.key - ch.key, ch.right, path))