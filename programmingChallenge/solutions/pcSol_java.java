import java.util.*;

/**
 * Computes the k-th shortest path from source to destination in a weighted directed graph
 * without allowing two paths to arrive with the same cost, using a simplified Eppstein algorithm.
 */
public class pcSol_java {
    // A large 'infinity' value for unreachable distances
    static final long INF = (long) 1e18;

    /**
     * Represents a weighted edge to a neighbor in the graph
     */
    static class Edge {
        int to;         // Destination vertex
        long weight;    // Edge weight

        Edge(int to, long weight) {
            this.to = to;
            this.weight = weight;
        }
    }

    /**
     * Node in a leftist heap storing a "sidetrack" deviation
     * from the shortest-path tree with its extra cost (key).
     */
    static class HeapNode {
        long key;          // Extra cost of taking this sidetrack edge
        int origin, value; // origin = starting vertex, value = destination of sidetrack
        HeapNode left, right; // Left and right children in the heap
        int rank;          // Null-path length for leftist-heap balancing

        HeapNode(long key, int origin, int value, HeapNode left, HeapNode right) {
            this.key = key;
            this.origin = origin;
            this.value = value;
            this.left = left;
            this.right = right;
            // Rank = 1 + min(rank(left), rank(right))
            this.rank = 1 + Math.min(rank(left), rank(right));
        }

        /**
         * Returns the rank of a node, or 0 if null
         */
        static int rank(HeapNode h) {
            return h == null ? 0 : h.rank;
        }

        /**
         * Inserts a new sidetrack (key, origin->value) into heap h
         * by merging with a newly created node.
         */
        static HeapNode insert(HeapNode h, long key, int origin, int value) {
            HeapNode node = new HeapNode(key, origin, value, null, null);
            return merge(h, node);
        }

        /**
         * Merges two leftist heaps a and b, keeping the smallest key at root.
         * Ensures leftist property by swapping children if needed.
         */
        static HeapNode merge(HeapNode a, HeapNode b) {
            if (a == null) return b;
            if (b == null) return a;
            // Ensure 'a' has the smaller key
            if (b.key < a.key) {
                HeapNode temp = a;
                a = b;
                b = temp;
            }
            // Merge b into the right subtree of a
            a.right = merge(a.right, b);
            // Enforce leftist property: left.rank >= right.rank
            if (rank(a.left) < rank(a.right)) {
                HeapNode temp = a.left;
                a.left = a.right;
                a.right = temp;
            }
            // Update rank after merge
            a.rank = 1 + rank(a.right);
            return a;
        }
    }

    /**
     * Runs Dijkstra's algorithm on reversed graph 'g' from source 'src'.
     * Fills 'pred' with the predecessor of each reachable node in the shortest-path tree.
     * Returns the distance array 'd'.
     */
    static long[] dijkstra(List<Edge>[] g, int src, int[] pred) {
        int n = g.length;
        long[] d = new long[n];
        Arrays.fill(d, INF);
        d[src] = 0;
        // Min-heap storing (distance, vertex)
        PriorityQueue<long[]> pq =
            new PriorityQueue<>(Comparator.comparingLong(a -> a[0]));
        pq.add(new long[]{0, src});

        while (!pq.isEmpty()) {
            long[] cur = pq.poll();
            long distU = cur[0];
            int u = (int) cur[1];
            // Skip stale entries
            if (distU != d[u]) continue;

            // Relax edges out of u
            for (Edge e : g[u]) {
                if (distU + e.weight < d[e.to]) {
                    d[e.to] = distU + e.weight;
                    pred[e.to] = u;
                    pq.add(new long[]{d[e.to], e.to});
                }
            }
        }
        return d;
    }

    /**
     * Computes the k-th smallest path distance and sidetrack sequence
     * from 'src' to 'dst', disallowing repeated arrival distances.
     * Returns a Result containing the distance and the list of sidetracks.
     */
    static class Result {
        long distance;
        List<int[]> sidetracks;
        Result(long distance, List<int[]> sidetracks) {
            this.distance = distance;
            this.sidetracks = sidetracks;
        }
    }

    static Result shortestPathsNoSameArrival(List<Edge>[] g, int src, int dst, int k) {
        int n = g.length;
        // 1) Build reversed graph for backward Dijkstra
        List<Edge>[] revg = new ArrayList[n];
        for (int i = 0; i < n; i++) revg[i] = new ArrayList<>();
        for (int u = 0; u < n; u++)
            for (Edge e : g[u])
                revg[e.to].add(new Edge(u, e.weight));

        // 2) Dijkstra from dst to get dist[] and pred[]
        int[] pred = new int[n];
        Arrays.fill(pred, -1);
        long[] d = dijkstra(revg, dst, pred);

        // If src unreachable, no k-th path exists
        if (d[src] == INF) return null;

        // 3) Build shortest-path tree from pred[]
        List<Integer>[] tree = new ArrayList[n];
        for (int i = 0; i < n; i++) tree[i] = new ArrayList<>();
        for (int u = 0; u < n; u++)
            if (pred[u] != -1)
                tree[pred[u]].add(u);

        // 4) Construct sidetrack heaps by BFS from dst
        HeapNode[] h = new HeapNode[n];
        Queue<Integer> queue = new LinkedList<>();
        queue.add(dst);
        while (!queue.isEmpty()) {
            int u = queue.poll();
            boolean seenPrimary = false;
            // Consider all outgoing edges as potential sidetracks
            for (Edge e : g[u]) {
                if (d[e.to] == INF) continue;
                long extraCost = e.weight + d[e.to] - d[u];
                // Skip the tree edge exactly once (extraCost==0)
                if (!seenPrimary && e.to == pred[u] && extraCost == 0) {
                    seenPrimary = true;
                    continue;
                }
                h[u] = HeapNode.insert(h[u], extraCost, u, e.to);
            }
            // Propagate heap to tree children
            for (int c : tree[u]) {
                h[c] = h[u];
                queue.add(c);
            }
        }

        // 5) Extract the k distances using a min-heap over sidetracks
        PriorityQueue<Object[]> pq =
            new PriorityQueue<>(Comparator.comparingLong(a -> (Long) a[0]));
        List<Result> results = new ArrayList<>();
        // Shortest path has no sidetracks
        results.add(new Result(d[src], new ArrayList<>()));

        // Seed first deviation (if any)
        if (h[src] != null)
            pq.add(new Object[]{d[src] + h[src].key, h[src], new ArrayList<int[]>()});

        // Collect up to k distinct arrival costs
        while (!pq.isEmpty() && results.size() < k) {
            Object[] cur = pq.poll();
            long cd = (Long) cur[0];
            HeapNode hn = (HeapNode) cur[1];
            @SuppressWarnings("unchecked")
            List<int[]> path = new ArrayList<>((List<int[]>) cur[2]);
            path.add(new int[]{hn.origin, hn.value});

            // Only record if strictly greater than last
            if (cd > results.get(results.size() - 1).distance)
                results.add(new Result(cd, path));

            // Expand further deviations
            if (h[hn.value] != null)
                pq.add(new Object[]{cd + h[hn.value].key, h[hn.value], path});
            if (hn.left != null)
                pq.add(new Object[]{cd + hn.left.key - hn.key, hn.left, cur[2]});
            if (hn.right != null)
                pq.add(new Object[]{cd + hn.right.key - hn.key, hn.right, cur[2]});
        }

        // If fewer than k paths found, return null
        if (results.size() < k) return null;
        return results.get(k - 1);
    }

    /**
     * Reconstructs the actual k-th shortest path as a list of vertices
     * by combining the sidetracks with the predecessor tree.
     */
    static List<Integer> getKthShortestPath(List<Edge>[] g, int src, int dst, int k) {
        int n = g.length;
        
        // Re-run reverse-Dijkstra to get pred[] for path reconstruction
        List<Edge>[] revg = new ArrayList[n];
        for (int i = 0; i < n; i++) revg[i] = new ArrayList<>();
        for (int u = 0; u < n; u++)
            for (Edge e : g[u])
                revg[e.to].add(new Edge(u, e.weight));

        int[] pred = new int[n];
        Arrays.fill(pred, -1);
        dijkstra(revg, dst, pred);

        // Get the k-th sidetrack result
        Result res = shortestPathsNoSameArrival(g, src, dst, k);
        if (res == null) return null;
        List<int[]> sidetracks = res.sidetracks;

        // Walk from src to dst, applying sidetracks when they match
        List<Integer> path = new ArrayList<>();
        int u = src, idx = 0;
        while (u != dst) {
            path.add(u);
            if (idx < sidetracks.size() && sidetracks.get(idx)[0] == u) {
                // take the sidetrack
                u = sidetracks.get(idx)[1];
                idx++;
            } else {
                // follow the shortest-path tree
                u = pred[u];
            }
        }
        path.add(dst);
        return path;
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        // Read n=#vertices, m=#edges, s=source, t=destination, k=#paths
        int n = sc.nextInt(), m = sc.nextInt(), s = sc.nextInt(), t = sc.nextInt(), k = sc.nextInt();
        
        // Build adjacency list for forward graph
        List<Edge>[] g = new ArrayList[n];
        for (int i = 0; i < n; i++) g[i] = new ArrayList<>();
        for (int i = 0; i < m; i++) {
            int u = sc.nextInt(), v = sc.nextInt();
            long w = sc.nextLong();
            g[u].add(new Edge(v, w));
        }

        // Compute k-th path and print it (or -1 if none)
        List<Integer> path = getKthShortestPath(g, s, t, k);
        if (path == null || path.isEmpty()) {
            System.out.println(-1);
        } else {
            for (int i = 0; i < path.size(); i++) {
                System.out.print(path.get(i));
                if (i != path.size() - 1) System.out.print(" ");
            }
            System.out.println();
        }
    }
}
