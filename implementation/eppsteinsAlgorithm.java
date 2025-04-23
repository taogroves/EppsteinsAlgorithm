import java.util.*;

class EHeap {
    int rank, key, value;
    EHeap left, right;

    EHeap(int rank, int key, int value, EHeap left, EHeap right) {
        this.rank = rank;
        this.key = key;
        this.value = value;
        this.left = left;
        this.right = right;
    }

    static EHeap insert(EHeap a, int k, int v) {
        if (a == null || k < a.key) {
            return new EHeap(1, k, v, a, null);
        }
        EHeap l = a.left;
        EHeap r = insert(a.right, k, v);
        if (l == null || (r != null && r.rank > l.rank)) {
            EHeap temp = l;
            l = r;
            r = temp;
        }
        int newRank = (r != null) ? r.rank + 1 : 1;
        return new EHeap(newRank, a.key, a.value, l, r);
    }
}

public class eppsteinsAlgorithm {
    static final long INF = (long) 1e18;

    static long[] dijkstra(List<List<int[]>> g, int src, int[] parent) {
        int n = g.size();
        long[] dist = new long[n];
        Arrays.fill(dist, INF);
        dist[src] = 0;
        PriorityQueue<long[]> pq = new PriorityQueue<>(Comparator.comparingLong(a -> a[0]));
        pq.add(new long[]{0, src});
        while (!pq.isEmpty()) {
            long[] curr = pq.poll();
            long d_u = curr[0];
            int u = (int) curr[1];
            if (d_u != dist[u]) continue;
            for (int[] edge : g.get(u)) {
                int v = edge[1];
                long w = edge[0];
                if (d_u + w < dist[v]) {
                    dist[v] = d_u + w;
                    parent[v] = u;
                    pq.add(new long[]{dist[v], v});
                }
            }
        }
        return dist;
    }

    static List<Long> eppstein(List<List<int[]>> g, int src, int dst, int k) {
        int n = g.size();
        List<List<int[]>> revg = new ArrayList<>();
        for (int i = 0; i < n; i++) revg.add(new ArrayList<>());
        for (int u = 0; u < n; u++) {
            for (int[] edge : g.get(u)) {
                int w = edge[0], v = edge[1];
                revg.get(v).add(new int[]{w, u});
            }
        }

        int[] p = new int[n];
        Arrays.fill(p, -1);
        long[] d = dijkstra(revg, dst, p);

        if (d[src] == INF) return Collections.emptyList();

        List<List<Integer>> tree = new ArrayList<>();
        for (int i = 0; i < n; i++) tree.add(new ArrayList<>());
        for (int i = 0; i < n; i++) {
            if (p[i] != -1) tree.get(p[i]).add(i);
        }

        EHeap[] h = new EHeap[n];
        Queue<Integer> queue = new LinkedList<>();
        queue.add(dst);

        while (!queue.isEmpty()) {
            int u = queue.poll();
            boolean seenParent = false;
            for (int[] edge : g.get(u)) {
                int w = edge[0], v = edge[1];
                if (d[v] == INF) continue;
                long costDiff = w + d[v] - d[u];
                if (!seenParent && v == p[u] && costDiff == 0) {
                    seenParent = true;
                    continue;
                }
                h[u] = EHeap.insert(h[u], (int) costDiff, v);
            }
            for (int v : tree.get(u)) {
                h[v] = h[u];
                queue.add(v);
            }
        }

        List<Long> result = new ArrayList<>();
        result.add(d[src]);
        if (h[src] == null) return result;

        PriorityQueue<Object[]> heap = new PriorityQueue<>(Comparator.comparingLong(a -> (Long) a[0]));
        heap.add(new Object[]{d[src] + h[src].key, h[src]});

        while (!heap.isEmpty() && result.size() < k) {
            Object[] top = heap.poll();
            long totalCost = (long) top[0];
            EHeap node = (EHeap) top[1];
            result.add(totalCost);
            if (h[node.value] != null)
                heap.add(new Object[]{totalCost + h[node.value].key, h[node.value]});
            if (node.left != null)
                heap.add(new Object[]{totalCost + node.left.key - node.key, node.left});
            if (node.right != null)
                heap.add(new Object[]{totalCost + node.right.key - node.key, node.right});
        }

        return result;
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        System.out.println("Enter the number of edges:");
        int numEdges = sc.nextInt();

        Map<String, Integer> nodeMap = new HashMap<>();
        Map<Integer, String> reverseMap = new HashMap<>();
        List<int[]> edgesRaw = new ArrayList<>();
        int nextId = 0;

        System.out.println("Enter each edge in the format 'from to weight' (e.g., A B 3):");
        for (int i = 0; i < numEdges; i++) {
            String uStr = sc.next();
            String vStr = sc.next();
            int w = sc.nextInt();

            nodeMap.putIfAbsent(uStr, nextId);
            reverseMap.putIfAbsent(nextId, uStr);
            int u = nodeMap.get(uStr);
            if (!nodeMap.containsKey(vStr)) {
                nextId++;
                nodeMap.put(vStr, nextId);
                reverseMap.put(nextId, vStr);
            }
            int v = nodeMap.get(vStr);
            edgesRaw.add(new int[]{u, v, w});
        }

        int n = nodeMap.size();
        List<List<int[]>> g = new ArrayList<>();
        for (int i = 0; i < n; i++) g.add(new ArrayList<>());
        for (int[] edge : edgesRaw) g.get(edge[0]).add(new int[]{edge[2], edge[1]});

        System.out.print("Enter the source node: ");
        String sourceStr = sc.next();
        System.out.print("Enter the target node: ");
        String targetStr = sc.next();
        System.out.print("Enter the number of paths (k): ");
        int k = sc.nextInt();

        if (!nodeMap.containsKey(sourceStr) || !nodeMap.containsKey(targetStr)) {
            System.out.printf("No path found from %s to %s.\n", sourceStr, targetStr);
            return;
        }

        int source = nodeMap.get(sourceStr);
        int target = nodeMap.get(targetStr);
        List<Long> results = eppstein(g, source, target, k);

        if (results.isEmpty()) {
            System.out.printf("No path found from %s to %s.\n", sourceStr, targetStr);
        } else {
            for (int i = 0; i < results.size(); i++) {
                System.out.printf("Path %d: %d\n", i + 1, results.get(i));
            }
            for (int i = results.size(); i < k; i++) {
                System.out.println("-1");
            }
        }
    }
}
