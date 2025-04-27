#include <iostream>
#include <vector>
#include <queue>       // Provides priority_queue and queue
#include <tuple>       // Provides tuple for storing states
#include <limits>      // Provides numeric_limits for INF
#include <functional>  // Provides greater<> comparator
#include <algorithm>   // Provides swap

using namespace std;

// compiles with: g++ -std=c++17 -O2 -o a.out pcSol_cpp.cpp

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
// INF represents an effectively infinite distance (half of max long long)
static const long long INF = numeric_limits<long long>::max() / 2;

//------------------------------------------------------------------------------
// Dijkstra's Algorithm (Backward)
//------------------------------------------------------------------------------
/**
 * Runs Dijkstra's algorithm on a reversed graph to compute shortest distances
 * from a given source 'src' to all other nodes.
 *
 * @param g   The reversed adjacency list: g[u] contains (weight, v) pairs
 * @param src The source vertex index
 * @return    A pair of vectors:
 *            - first:  d[u] = shortest distance from src to u
 *            - second: parent[u] = preceding vertex on the shortest path
 */
pair<vector<long long>, vector<int>> dijkstra(
    const vector<vector<pair<long long,int>>>& g,
    int src
) {
    int n = g.size();
    vector<long long> d(n, INF);
    vector<int> parent(n, -1);
    d[src] = 0;  // Distance to source is zero

    // Min-heap (priority_queue) storing (distance, vertex)
    priority_queue<
        pair<long long,int>,
        vector<pair<long long,int>>,
        greater<>
    > pq;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [distU, u] = pq.top();
        pq.pop();

        // If this entry is outdated (greater than current best), skip it
        if (distU != d[u]) continue;

        // Relax all outgoing edges from u
        for (auto &edge : g[u]) {
            long long w = edge.first;  // edge weight
            int v = edge.second;       // neighbor
            long long newDist = distU + w;
            if (newDist < d[v]) {
                d[v] = newDist;
                parent[v] = u;
                pq.push({newDist, v});
            }
        }
    }

    return {d, parent};
}

//------------------------------------------------------------------------------
// Leftist Heap Node Definition
//------------------------------------------------------------------------------
/**
 * A node in a leftist heap storing "sidetrack" edges for Eppstein's algorithm.
 * Each node keeps the following:
 *  - rank: null-path length for maintaining leftist property
 *  - key:  cost difference of this sidetrack edge
 *  - origin: source vertex where this sidetrack originates
 *  - value: destination vertex of this sidetrack
 *  - left, right: child pointers
 */
struct EHeap {
    long long rank;
    long long key;
    int origin;
    int value;
    EHeap *left;
    EHeap *right;

    /**
     * Constructs a new heap node.
     * @param r    null-path length
     * @param k    cost difference (heap key)
     * @param o    origin vertex
     * @param v    destination vertex
     * @param l    left child
     * @param rr   right child
     */
    EHeap(long long r, long long k, int o, int v, EHeap* l, EHeap* rr)
        : rank(r), key(k), origin(o), value(v), left(l), right(rr) {}

    /**
     * Inserts a new sidetrack into the heap persistently.
     * Ensures the min-heap and leftist properties.
     *
     * @param a    Current heap root (or nullptr)
     * @param k    cost difference to insert
     * @param o    origin vertex
     * @param v    destination vertex
     * @return     New heap root with the new element
     */
    static EHeap* insertNode(
        EHeap* a,
        long long k,
        int o,
        int v
    ) {
        // If empty or new key is smaller than root key, make new root
        if (!a || k < a->key) {
            return new EHeap(1, k, o, v, a, nullptr);
        }

        // Otherwise, recursively insert into right subtree
        a->right = insertNode(a->right, k, o, v);

        // Enforce leftist property: left child rank >= right child rank
        if (!a->left || (a->right && a->right->rank > a->left->rank)) {
            swap(a->left, a->right);
        }

        // Update this node's rank: one plus rank of right child
        a->rank = (a->right ? a->right->rank : 0) + 1;
        return a;
    }
};

//------------------------------------------------------------------------------
// Eppstein's K-Shortest Paths (No Same Arrival)
//------------------------------------------------------------------------------
/**
 * Computes up to k shortest path costs from src to dst without repeating
 * any arrival edge, using Eppstein's algorithm with persistent sidetrack heaps.
 *
 * @param g   Forward adjacency list: g[u] contains (weight, v)
 * @param src Source vertex index
 * @param dst Destination vertex index
 * @param k   Number of paths to compute
 * @return    Vector of pairs (cost, sidetrack sequence) sorted by cost
 */
vector<pair<long long, vector<pair<int,int>>>>
shortest_paths_no_same_arrival(
    const vector<vector<pair<long long,int>>>& g,
    int src,
    int dst,
    int k
) {
    int n = g.size();

    // 1. Build reversed graph for backward Dijkstra
    vector<vector<pair<long long,int>>> revg(n);
    for (int u = 0; u < n; ++u) {
        for (auto &e : g[u]) {
            int v = e.second;
            long long w = e.first;
            revg[v].push_back({w, u});
        }
    }

    // 2. Run Dijkstra from dst on reversed graph to get dist[] and parent[]
    auto [d, parent] = dijkstra(revg, dst);
    if (d[src] == INF) {
        // No path exists at all
        return {};
    }

    // 3. Build shortest-path tree from parent pointers
    vector<vector<int>> tree(n);
    for (int u = 0; u < n; ++u) {
        int p = parent[u];
        if (p != -1) {
            tree[p].push_back(u);
        }
    }

    // 4. Construct persistent leftist heaps of sidetrack edges for each node
    vector<EHeap*> heaps(n, nullptr);
    queue<int> q;
    q.push(dst);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        bool skippedTreeEdge = false;

        // Insert all non-tree edges as sidetracks
        for (auto &e : g[u]) {
            int v = e.second;
            long long w = e.first;
            if (d[v] == INF) continue;  // skip unreachable

            long long costDiff = w + d[v] - d[u];
            // Skip the primary tree edge exactly once
            if (!skippedTreeEdge && v == parent[u] && costDiff == 0) {
                skippedTreeEdge = true;
                continue;
            }
            // Persistent insert into heap at u
            heaps[u] = EHeap::insertNode(heaps[u], costDiff, u, v);
        }

        // Propagate heap pointer to children in the shortest-path tree
        for (int child : tree[u]) {
            heaps[child] = heaps[u];
            q.push(child);
        }
    }

    // 5. Extract k shortest path costs using the sidetrack heaps
    // Store (cost, sidetrack path) in 'results'
    vector<pair<long long, vector<pair<int,int>>>> results;
    results.emplace_back(d[src], vector<pair<int,int>>());
    if (!heaps[src]) {
        // No sidetracks => only single shortest path
        return results;
    }

    // Priority queue over (totalCost, heapNode, sidetrackSequence)
    using State = tuple<long long, EHeap*, vector<pair<int,int>>>;
    priority_queue<State, vector<State>, greater<>> pq2;

    // Seed with first sidetrack at the source
    pq2.push({d[src] + heaps[src]->key, heaps[src], {}});

    while (!pq2.empty() && results.size() < (size_t)k) {
        auto [costSoFar, node, path] = pq2.top();
        pq2.pop();

        // Extend path by the current sidetrack
        vector<pair<int,int>> newPath = path;
        newPath.emplace_back(node->origin, node->value);

        // Only record if cost strictly increases to avoid duplicates
        if (costSoFar > results.back().first) {
            results.emplace_back(costSoFar, newPath);
        }

        // Expand along this sidetrack node
        if (heaps[node->value]) {
            pq2.push({
                costSoFar + heaps[node->value]->key,
                heaps[node->value],
                newPath
            });
        }
        // Also explore left and right children in the heap
        if (node->left)  pq2.push({costSoFar + node->left->key  - node->key,  node->left,  path});
        if (node->right) pq2.push({costSoFar + node->right->key - node->key, node->right, path});
    }

    return results;
}

//------------------------------------------------------------------------------
// Main Program Entry
//------------------------------------------------------------------------------
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Read graph parameters:
    // n = #vertices, m = #edges, s = source, t = target, k = #paths to find
    int n, m, s, t, k;
    cin >> n >> m >> s >> t >> k;

    // Build adjacency list for the forward graph
    vector<vector<pair<long long,int>>> graph(n);
    for (int i = 0; i < m; ++i) {
        int u, v;
        long long w;
        cin >> u >> v >> w;
        graph[u].emplace_back(w, v);
    }

    // Compute the k shortest paths with no same arrival edge
    auto paths = shortest_paths_no_same_arrival(graph, s, t, k);

    // Output the cost of the k-th path (or -1 if none exist)
    if (paths.size() < (size_t)k) {
        cout << "-1\n";
    }

    // Re-run Dijkstra to get dist and parent arrays
    vector<vector<pair<long long,int>>> revg(n);
    for (int u = 0; u < n; ++u) {
        for (auto &e : graph[u]) {
            int v = e.second;
            long long w = e.first;
            revg[v].emplace_back(w, u);
        }
    }
    auto [d, parent] = dijkstra(revg, t);

    // Reconstruct path
    vector<int> resultPath;
    int current = s;
    size_t sidetrackIndex = 0;
    auto &sidetracks = paths.back().second; // Get sidetrack sequence

    while (current != t) {
        // Follow shortest path until sidetrack needed
        while (true) {
            // If sidetrack available and matches current node
            resultPath.push_back(current);
            if (sidetrackIndex < sidetracks.size() && sidetracks[sidetrackIndex].first == current) {
                current = sidetracks[sidetrackIndex].second;
                sidetrackIndex++;
                break;
            }
            current = parent[current];
        }
    }
    resultPath.push_back(t);

    // Output the path
    for (size_t i = 0; i < resultPath.size(); ++i) {
        cout << resultPath[i];
        if (i + 1 < resultPath.size()) cout << ' ';
    }
    cout << '\n';
}