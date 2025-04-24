import java.util.*;

public class pcSol_java {
    // A large constant to represent infinity (unreachable distance)
    static final long INF = (long) 1e18;

    // Simple directed edge with target node and weight
    static class Edge {
        int to;          // endpoint of the directed edge
        long weight;     // cost to traverse this edge
        Edge(int to, long weight) {
            this.to = to;
            this.weight = weight;
        }
    }

    // Leftist heap node to manage "deviation" options beyond the shortest path
    static class HeapNode {
        long key;        // additional cost when taking this deviation
        int origin;      // node where the deviation starts
        int value;       // node to which this deviation leads
        HeapNode left, right; // children in the leftist heap
        int rank;        // distance to the nearest null child

        // Constructor initializes fields and computes rank
        HeapNode(long key, int origin, int value, HeapNode left, HeapNode right) {
            this.key = key;
            this.origin = origin;
            this.value = value;
            this.left = left;
            this.right = right;
            this.rank = 1 + Math.min(rank(left), rank(right));
        }

        // Returns the rank of a node (0 if null)
        static int rank(HeapNode h) {
            return h == null ? 0 : h.rank;
        }

        // Insert a new deviation node into heap h
        static HeapNode insert(HeapNode h, long key, int origin, int value) {
            HeapNode node = new HeapNode(key, origin, value, null, null);
            return merge(h, node);
        }

        // Merge two leftist heaps, maintaining the smaller key at the root
        static HeapNode merge(HeapNode a, HeapNode b) {
            if (a == null) return b;
            if (b == null) return a;
            // ensure a has the smaller key
            if (b.key < a.key) {
                HeapNode temp = a;
                a = b;
                b = temp;
            }
            // merge b into the right subtree of a
            a.right = merge(a.right, b);
            // enforce leftist property: left.rank >= right.rank
            if (rank(a.left) < rank(a.right)) {
                HeapNode tmp = a.left;
                a.left = a.right;
                a.right = tmp;
            }
            // update rank
            a.rank = 1 + rank(a.right);
            return a;
        }
    }

    /**
     * Dijkstra's algorithm on graph g from source src.
     * Fills pred[] with the predecessor of each node on the shortest-path tree.
     * Returns array d[] where d[u] is the distance from src to u.
     */
    static long[] dijkstra(List<Edge>[] g, int src, int[] pred) {
        int n = g.length;
        long[] d = new long[n];
        Arrays.fill(d, INF);
        d[src] = 0;
        PriorityQueue<long[]> pq = new PriorityQueue<>(Comparator.comparingLong(a -> a[0]));
        pq.add(new long[]{0, src});

        while (!pq.isEmpty()) {
            long[] cur = pq.poll();
            long du = cur[0];
            int u = (int) cur[1];
            // ignore stale entries
            if (du != d[u]) continue;
            for (Edge e : g[u]) {
                long nd = du + e.weight;
                if (nd < d[e.to]) {
                    d[e.to] = nd;
                    pred[e.to] = u;  // record tree edge
                    pq.add(new long[]{nd, e.to});
                }
            }
        }
        return d;
    }

    /**
     * Returns the k-th shortest path distance from src to dst,
     * disallowing two paths that arrive at the same total cost.
     * Uses a shortest-path tree + deviation heaps (Yen's algorithm variant).
     */
    static long shortestPathsNoSameArrival(List<Edge>[] g, int src, int dst, int k) {
        int n = g.length;

        // Build reverse graph to run Dijkstra from dst back to all nodes
        List<Edge>[] revg = new ArrayList[n];
        for (int i = 0; i < n; i++) revg[i] = new ArrayList<>();
        for (int u = 0; u < n; u++)
            for (Edge e : g[u])
                revg[e.to].add(new Edge(u, e.weight));

        // dist[]: distance from dst to each u; pred[]: predecessor on shortest-path tree
        int[] pred = new int[n];
        Arrays.fill(pred, -1);
        long[] d = dijkstra(revg, dst, pred);
        if (d[src] == INF) return -1;  // no path exists

        // Build tree adjacency: from each node to its children in the shortest-path tree
        List<Integer>[] tree = new ArrayList[n];
        for (int i = 0; i < n; i++) tree[i] = new ArrayList<>();
        for (int u = 0; u < n; u++)
            if (pred[u] != -1)
                tree[pred[u]].add(u);

        // Build deviation heaps: for each node, gather edges not in the tree (or extra-cost)
        HeapNode[] h = new HeapNode[n];
        Queue<Integer> queue = new LinkedList<>();
        queue.add(dst);
        while (!queue.isEmpty()) {
            int u = queue.poll();
            boolean seenTreeEdge = false;
            for (Edge e : g[u]) {
                if (d[e.to] == INF) continue;  // skip unreachable
                long extra = e.weight + d[e.to] - d[u];
                // skip the one tree edge that has zero extra cost
                if (!seenTreeEdge && e.to == pred[u] && extra == 0) {
                    seenTreeEdge = true;
                    continue;
                }
                // insert deviation into heap for u
                h[u] = HeapNode.insert(h[u], extra, u, e.to);
            }
            // propagate the same heap to children in the shortest-path tree
            for (int v : tree[u]) {
                h[v] = h[u];
                queue.add(v);
            }
        }

        // Collect k distinct path lengths in increasing order
        List<Long> results = new ArrayList<>();
        results.add(d[src]);  // the shortest path
        PriorityQueue<Object[]> pq = new PriorityQueue<>(Comparator.comparingLong(a -> (Long) a[0]));
        if (h[src] != null)
            pq.add(new Object[]{d[src] + h[src].key, h[src]});

        while (!pq.isEmpty() && results.size() < k) {
            Object[] cur = pq.poll();
            long cost = (Long) cur[0];
            HeapNode node = (HeapNode) cur[1];

            // only add if strictly larger than last recorded cost
            if (cost > results.get(results.size() - 1))
                results.add(cost);

            // push further deviations from this path
            if (h[node.value] != null)
                pq.add(new Object[]{cost + h[node.value].key, h[node.value]});
            if (node.left != null)
                pq.add(new Object[]{cost + node.left.key - node.key, node.left});
            if (node.right != null)
                pq.add(new Object[]{cost + node.right.key - node.key, node.right});
        }

        return results.get(results.size() - 1);
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int n = sc.nextInt(), m = sc.nextInt(), s = sc.nextInt(), t = sc.nextInt(), k = sc.nextInt();
        List<Edge>[] g = new ArrayList[n];
        for (int i = 0; i < n; i++) g[i] = new ArrayList<>();
        for (int i = 0; i < m; i++) {
            int u = sc.nextInt(), v = sc.nextInt();
            long w = sc.nextLong();
            g[u].add(new Edge(v, w));
        }
        System.out.println(shortestPathsNoSameArrival(g, s, t, k));
        sc.close();
    }
}
