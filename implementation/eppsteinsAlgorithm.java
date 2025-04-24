import java.util.*;

/**
 * Leftist heap node for storing sidetrack edges in Eppstein's algorithm.
 */
class EHeap {
    int rank;         // leftist heap rank
    long key;         // cost difference (sidetrack edge weight)
    int value;        // target vertex of the sidetrack edge
    EHeap left, right;

    /**
     * Constructor: create a new heap node.
     * @param rank heap rank
     * @param key  cost difference
     * @param value target vertex
     * @param left  left subtree
     * @param right right subtree
     */
    EHeap(int rank, long key, int value, EHeap left, EHeap right) {
        this.rank = rank;
        this.key  = key;
        this.value = value;
        this.left  = left;
        this.right = right;
    }

    /**
     * Inserts a new (key, value) pair into the leftist heap persistently.
     * @param a existing heap or null
     * @param k cost difference to insert
     * @param v target vertex
     * @return new heap with the inserted node
     */
    static EHeap insert(EHeap a, long k, int v) {
        // Base: empty heap or new key is smaller than root -> make new root
        if (a == null || k < a.key) {
            return new EHeap(1, k, v, a, null);
        }
        // Recurse into right subtree
        EHeap l = a.left;
        EHeap r = insert(a.right, k, v);
        // Ensure leftist property: left.rank >= right.rank
        if (l == null || (r != null && r.rank > l.rank)) {
            EHeap tmp = l;
            l = r;
            r = tmp;
        }
        // Compute new rank = 1 + rank of right child
        int newRank = (r != null) ? r.rank + 1 : 1;
        // Return new node retaining original key/value with updated children
        return new EHeap(newRank, a.key, a.value, l, r);
    }
}

/**
 * Implements Eppstein's k‑shortest paths algorithm.
 */
public class eppsteinsAlgorithm {
    static final long INF = (long)1e18;  // infinity sentinel for distances

    /**
     * Standard Dijkstra to compute shortest distances from src in graph g.
     * Also fills parent[] to reconstruct the shortest path tree.
     * @param g adjacency list: g[u] holds pairs (weight, v)
     * @param src source vertex index
     * @param parent array to record tree parent of each node
     * @return dist[] array of shortest distances from dst back to all nodes
     */
    static long[] dijkstra(List<List<int[]>> g, int src, int[] parent) {
        int n = g.size();
        long[] dist = new long[n];
        Arrays.fill(dist, INF);
        dist[src] = 0;
        // Min‑heap over (dist, vertex)
        PriorityQueue<long[]> pq = new PriorityQueue<>(Comparator.comparingLong(a -> a[0]));
        pq.add(new long[]{0, src});
        while (!pq.isEmpty()) {
            long[] cur = pq.poll();
            long d_u = cur[0];
            int u = (int)cur[1];
            if (d_u != dist[u]) continue;  // stale entry
            for (int[] e : g.get(u)) {
                int v = e[1];
                int w = e[0];
                // Relax edge u->v
                if (d_u + w < dist[v]) {
                    dist[v] = d_u + w;
                    parent[v] = u;
                    pq.add(new long[]{dist[v], v});
                }
            }
        }
        return dist;
    }

    /**
     * Computes the k shortest path costs from src to dst.
     * @param g adjacency list of the forward graph
     * @param src source index
     * @param dst destination index
     * @param k  number of paths to retrieve
     * @return list of the k shortest path costs (sorted)
     */
    static List<Long> eppstein(List<List<int[]>> g, int src, int dst, int k) {
        int n = g.size();
        // Build reverse graph for backward Dijkstra
        List<List<int[]>> revg = new ArrayList<>();
        for (int i = 0; i < n; i++) revg.add(new ArrayList<>());
        for (int u = 0; u < n; u++)
            for (int[] e : g.get(u))
                revg.get(e[1]).add(new int[]{e[0], u});

        // Run Dijkstra from dst in reverse graph to get dist[] and tree parents
        int[] parent = new int[n];
        Arrays.fill(parent, -1);
        long[] d = dijkstra(revg, dst, parent);
        if (d[src] == INF) return Collections.emptyList();  // no path

        // Build the shortest‑path tree: tree[u] = list of children of u
        List<List<Integer>> tree = new ArrayList<>();
        for (int i = 0; i < n; i++) tree.add(new ArrayList<>());
        for (int i = 0; i < n; i++) {
            if (parent[i] != -1) {
                tree.get(parent[i]).add(i);
            }
        }

        // Build leftist heaps of "sidetrack" edges for each node
        EHeap[] h = new EHeap[n];
        Queue<Integer> q = new LinkedList<>();
        q.add(dst);
        while (!q.isEmpty()) {
            int u = q.poll();
            boolean seenPar = false;
            for (int[] e : g.get(u)) {
                int v = e[1];
                long w = e[0];
                if (d[v] == INF) continue;  // unreachable
                long costDiff = w + d[v] - d[u];
                // skip the tree edge
                if (!seenPar && v == parent[u] && costDiff == 0) {
                    seenPar = true;
                } else {
                    // insert sidetrack
                    h[u] = EHeap.insert(h[u], costDiff, v);
                }
            }
            // propagate heap pointer to children in tree
            for (int v : tree.get(u)) {
                h[v] = h[u];
                q.add(v);
            }
        }

        // Now extract up to k shortest path costs
        List<Long> result = new ArrayList<>();
        result.add(d[src]);            // shortest path
        if (h[src] == null) return result;  // no sidetracks -> only one path

        // Min‑heap over (totalCost, pointer into sidetrack heap)
        PriorityQueue<Object[]> heap = new PriorityQueue<>(Comparator.comparingLong(a -> (Long)a[0]));
        heap.add(new Object[]{d[src] + h[src].key, h[src]});

        while (!heap.isEmpty() && result.size() < k) {
            Object[] top = heap.poll();
            long costSoFar = (long)top[0];
            EHeap node    = (EHeap)top[1];
            result.add(costSoFar);

            // expand this sidetrack node
            if (h[node.value] != null) {
                heap.add(new Object[]{costSoFar + h[node.value].key, h[node.value]});
            }
            // consider next siblings in leftist heap
            if (node.left != null) {
                heap.add(new Object[]{costSoFar + node.left.key - node.key, node.left});
            }
            if (node.right != null) {
                heap.add(new Object[]{costSoFar + node.right.key - node.key, node.right});
            }
        }
        return result;
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        System.out.print("Enter the number of edges: ");
        int numEdges = sc.nextInt();

        // Map string labels to integer IDs
        Map<String,Integer> nodeMap = new HashMap<>();
        Map<Integer,String> revMap  = new HashMap<>();
        List<int[]> edgesRaw        = new ArrayList<>();
        int nextId = 0;

        System.out.println("Enter edges as: from to weight");
        for (int i = 0; i < numEdges; i++) {
            String uStr = sc.next(), vStr = sc.next();
            int w = sc.nextInt();
            // assign unique IDs
            if (!nodeMap.containsKey(uStr)) {
                nodeMap.put(uStr, nextId);
                revMap.put(nextId, uStr);
                nextId++;
            }
            if (!nodeMap.containsKey(vStr)) {
                nodeMap.put(vStr, nextId);
                revMap.put(nextId, vStr);
                nextId++;
            }
            int u = nodeMap.get(uStr), v = nodeMap.get(vStr);
            edgesRaw.add(new int[]{u, v, w});
        }

        // Build adjacency list
        int n = nextId;
        List<List<int[]>> g = new ArrayList<>();
        for (int i = 0; i < n; i++) g.add(new ArrayList<>());
        for (int[] e : edgesRaw) {
            g.get(e[0]).add(new int[]{e[2], e[1]});
        }

        // Read source, target, and k
        System.out.print("Source: ");
        String sourceStr = sc.next();
        System.out.print("Target: ");
        String targetStr = sc.next();
        System.out.print("How many paths (k): ");
        int k = sc.nextInt();

        // Validate labels
        if (!nodeMap.containsKey(sourceStr) || !nodeMap.containsKey(targetStr)) {
            System.out.printf("No path found from %s to %s.%n", sourceStr, targetStr);
            sc.close();
            return;
        }

        int src = nodeMap.get(sourceStr), dst = nodeMap.get(targetStr);
        List<Long> paths = eppstein(g, src, dst, k);

        // Output results, padding with -1
        if (paths.isEmpty()) {
            System.out.printf("No path found from %s to %s.%n", sourceStr, targetStr);
        } else {
            for (int i = 0; i < paths.size(); i++) {
                System.out.printf("Path %d: %d%n", i+1, paths.get(i));
            }
            for (int i = paths.size(); i < k; i++) {
                System.out.println("-1");
            }
        }
        sc.close();
    }
}
