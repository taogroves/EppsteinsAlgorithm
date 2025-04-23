import java.util.*;

public class pcSol_java {
    static final long INF = (long) 1e18;

    static class Edge {
        int to;
        long weight;
        Edge(int to, long weight) {
            this.to = to;
            this.weight = weight;
        }
    }

    static class HeapNode {
        long key;
        int origin, value;
        HeapNode left, right;
        int rank;

        HeapNode(long key, int origin, int value, HeapNode left, HeapNode right) {
            this.key = key;
            this.origin = origin;
            this.value = value;
            this.left = left;
            this.right = right;
            this.rank = 1 + Math.min(rank(left), rank(right));
        }

        static int rank(HeapNode h) {
            return h == null ? 0 : h.rank;
        }

        static HeapNode insert(HeapNode h, long key, int origin, int value) {
            HeapNode node = new HeapNode(key, origin, value, null, null);
            return merge(h, node);
        }

        static HeapNode merge(HeapNode a, HeapNode b) {
            if (a == null) return b;
            if (b == null) return a;
            if (b.key < a.key) {
                HeapNode temp = a;
                a = b;
                b = temp;
            }
            a.right = merge(a.right, b);
            if (rank(a.left) < rank(a.right)) {
                HeapNode temp = a.left;
                a.left = a.right;
                a.right = temp;
            }
            a.rank = 1 + rank(a.right);
            return a;
        }
    }

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
            if (du != d[u]) continue;
            for (Edge e : g[u]) {
                if (du + e.weight < d[e.to]) {
                    d[e.to] = du + e.weight;
                    pred[e.to] = u;
                    pq.add(new long[]{d[e.to], e.to});
                }
            }
        }
        return d;
    }

    static long shortestPathsNoSameArrival(List<Edge>[] g, int src, int dst, int k) {
        int n = g.length;
        List<Edge>[] revg = new ArrayList[n];
        for (int i = 0; i < n; i++) revg[i] = new ArrayList<>();
        for (int u = 0; u < n; u++)
            for (Edge e : g[u])
                revg[e.to].add(new Edge(u, e.weight));

        int[] pred = new int[n];
        Arrays.fill(pred, -1);
        long[] d = dijkstra(revg, dst, pred);

        if (d[src] == INF) return -1;

        List<Integer>[] tree = new ArrayList[n];
        for (int i = 0; i < n; i++) tree[i] = new ArrayList<>();
        for (int u = 0; u < n; u++)
            if (pred[u] != -1)
                tree[pred[u]].add(u);

        HeapNode[] h = new HeapNode[n];
        Queue<Integer> queue = new LinkedList<>();
        queue.add(dst);
        while (!queue.isEmpty()) {
            int u = queue.poll();
            boolean seenp = false;
            for (Edge e : g[u]) {
                if (d[e.to] == INF) continue;
                long c = e.weight + d[e.to] - d[u];
                if (!seenp && e.to == pred[u] && c == 0) {
                    seenp = true;
                    continue;
                }
                h[u] = HeapNode.insert(h[u], c, u, e.to);
            }
            for (int v : tree[u]) {
                h[v] = h[u];
                queue.add(v);
            }
        }

        PriorityQueue<Object[]> pq = new PriorityQueue<>(Comparator.comparingLong(a -> (Long) a[0]));
        List<Long> results = new ArrayList<>();
        results.add(d[src]);

        if (h[src] != null)
            pq.add(new Object[]{d[src] + h[src].key, h[src], new ArrayList<int[]>()});

        while (!pq.isEmpty() && results.size() < k) {
            Object[] cur = pq.poll();
            long cd = (Long) cur[0];
            HeapNode ch = (HeapNode) cur[1];
            @SuppressWarnings("unchecked")
            List<int[]> path = new ArrayList<>((List<int[]>) cur[2]);
            path.add(new int[]{ch.origin, ch.value});

            if (cd > results.get(results.size() - 1))
                results.add(cd);

            if (h[ch.value] != null)
                pq.add(new Object[]{cd + h[ch.value].key, h[ch.value], path});
            if (ch.left != null)
                pq.add(new Object[]{cd + ch.left.key - ch.key, ch.left, cur[2]});
            if (ch.right != null)
                pq.add(new Object[]{cd + ch.right.key - ch.key, ch.right, cur[2]});
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
    }
}