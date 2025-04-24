#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;

// Infinity constant for unreachable distances
const long long INF = numeric_limits<long long>::max() / 2;

/**
 * Leftist heap node for storing sidetrack edges in Eppstein's algorithm.
 * Maintains a min-heap property on 'key' (cost difference) and
 * the leftist property (null-path length in 'rank').
 */
struct EHeap {
    int rank;         // Null-path length for leftist heap
    long long key;    // Cost difference of this sidetrack edge
    int value;        // Target vertex index for this edge
    EHeap *left, *right; // Child pointers

    // Constructor to initialize a new heap node
    EHeap(int r, long long k, int v, EHeap* l, EHeap* rgt)
        : rank(r), key(k), value(v), left(l), right(rgt) {}
};

/**
 * Persistent insert into leftist min-heap.
 * @param a    Current heap root (can be nullptr)
 * @param k    Key (cost difference) to insert
 * @param v    Value (vertex index) to associate
 * @return     New heap root including the inserted node
 */
EHeap* insert(EHeap* a, long long k, int v) {
    // If empty or new key is smaller, create new root node
    if (!a || k < a->key)
        return new EHeap(1, k, v, a, nullptr);

    // Recursively insert into right subtree
    EHeap* leftChild = a->left;
    EHeap* rightChild = insert(a->right, k, v);

    // Enforce leftist property: leftChild->rank >= rightChild->rank
    if (!leftChild || (rightChild && rightChild->rank > leftChild->rank))
        swap(leftChild, rightChild);

    // Compute new rank = 1 + rank of right child
    int newRank = rightChild ? rightChild->rank + 1 : 1;

    // Return a new node with preserved key/value and updated children
    return new EHeap(newRank, a->key, a->value, leftChild, rightChild);
}

/**
 * Runs Dijkstra's algorithm on graph g from source 'src'.
 * @param g       Adjacency list: g[u] contains pairs (weight, v)
 * @param src     Source vertex index
 * @param parent  Output vector to store parent pointers in shortest-path tree
 * @return        Vector of shortest distances from src to each vertex
 */
vector<long long> dijkstra(const vector<vector<pair<int,int>>>& g,
                           int src,
                           vector<int>& parent) {
    int n = g.size();
    vector<long long> dist(n, INF);
    dist[src] = 0;
    parent.assign(n, -1);

    // Min-heap over (distance, vertex)
    priority_queue<pair<long long,int>,
                   vector<pair<long long,int>>,
                   greater<>> pq;
    pq.emplace(0LL, src);

    while (!pq.empty()) {
        auto [d_u, u] = pq.top(); pq.pop();
        if (d_u != dist[u]) continue;  // Skip stale entries

        // Relax each outgoing edge u->v
        for (auto [w, v] : g[u]) {
            long long nd = d_u + w;
            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;
                pq.emplace(nd, v);
            }
        }
    }
    return dist;
}

/**
 * Computes the k shortest path costs from src to dst using Eppstein's algorithm.
 * @param g    Forward adjacency list
 * @param src  Source vertex index
 * @param dst  Destination vertex index
 * @param k    Number of shortest paths to compute
 * @return     Vector of up to k path costs in non-decreasing order
 */
vector<long long>
eppstein_k_shortest_paths(const vector<vector<pair<int,int>>>& g,
                            int src,
                            int dst,
                            int k) {
    int n = g.size();

    // Build the reverse graph for backward Dijkstra
    vector<vector<pair<int,int>>> revg(n);
    for (int u = 0; u < n; ++u) {
        for (auto [w, v] : g[u]) {
            revg[v].emplace_back(w, u);
        }
    }

    vector<int> parent;
    auto d = dijkstra(revg, dst, parent);
    if (d[src] == INF) return {};  // No path exists

    // Construct shortest-path tree from parent pointers
    vector<vector<int>> tree(n);
    for (int u = 0; u < n; ++u) {
        if (parent[u] != -1) {
            tree[parent[u]].push_back(u);
        }
    }

    // h[u] will point to a leftist heap of sidetrack edges from u
    vector<EHeap*> h(n, nullptr);
    queue<int> q;
    q.push(dst);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        bool seenPar = false;

        // For each outgoing edge u->v in forward graph
        for (auto [w, v] : g[u]) {
            if (d[v] == INF) continue;  // Skip unreachable nodes
            long long costDiff = w + d[v] - d[u];
            // Skip the tree edge once (the main shortest-path link)
            if (!seenPar && v == parent[u] && costDiff == 0) {
                seenPar = true;
            } else {
                // Insert this sidetrack edge into u's heap
                h[u] = insert(h[u], costDiff, v);
            }
        }

        // Propagate u's heap pointer to its children in the tree
        for (int v : tree[u]) {
            h[v] = h[u];
            q.push(v);
        }
    }

    // Collect the k shortest distances
    vector<long long> result;
    result.push_back(d[src]);  // The shortest path cost
    if (!h[src]) return result;  // No sidetracks => only one path

    // Priority queue over (totalCost, EHeap* node)
    using PQNode = pair<long long, EHeap*>;
    priority_queue<PQNode, vector<PQNode>, greater<>> pq;
    pq.emplace(d[src] + h[src]->key, h[src]);

    while (!pq.empty() && (int)result.size() < k) {
        auto [costSoFar, node] = pq.top(); pq.pop();
        result.push_back(costSoFar);

        // Continue along the main sidetrack chain
        if (h[node->value]) {
            pq.emplace(costSoFar + h[node->value]->key,
                       h[node->value]);
        }
        // Explore sibling sidetrack branches
        if (node->left) {
            pq.emplace(costSoFar + node->left->key - node->key,
                       node->left);
        }
        if (node->right) {
            pq.emplace(costSoFar + node->right->key - node->key,
                       node->right);
        }
    }

    return result;
}

int main() {
    cout << "Enter the number of edges:\n";
    int num_edges;
    cin >> num_edges;

    // Map string labels to integer IDs
    unordered_map<string,int> node_map;
    unordered_map<int,string> reverse_map;
    vector<tuple<int,int,int>> edges_raw;
    int next_id = 0;

    cout << "Enter each edge in the format 'from to weight' (e.g., A B 3):\n";
    for (int i = 0; i < num_edges; ++i) {
        string u_str, v_str;
        int w;
        cin >> u_str >> v_str >> w;

        // Assign unique IDs for each node label
        if (!node_map.count(u_str)) {
            node_map[u_str] = next_id;
            reverse_map[next_id] = u_str;
            ++next_id;
        }
        if (!node_map.count(v_str)) {
            node_map[v_str] = next_id;
            reverse_map[next_id] = v_str;
            ++next_id;
        }

        int u = node_map[u_str];
        int v = node_map[v_str];
        edges_raw.emplace_back(u, v, w);
    }

    // Build adjacency list for forward graph
    int n = next_id;
    vector<vector<pair<int,int>>> g(n);
    for (auto& [u, v, w] : edges_raw) {
        g[u].emplace_back(w, v);
    }

    // Read source, target, and k
    string source_str, target_str;
    int k;
    cout << "Enter the source node: ";
    cin >> source_str;
    cout << "Enter the target node: ";
    cin >> target_str;
    cout << "Enter the number of paths (k): ";
    cin >> k;

    // Validate inputs
    if (!node_map.count(source_str) || !node_map.count(target_str)) {
        cout << "No path found from " << source_str << " to " << target_str << ".\n";
        return 0;
    }

    int source = node_map[source_str];
    int target = node_map[target_str];

    auto results = eppstein_k_shortest_paths(g, source, target, k);

    // Output results, padding with -1 if fewer than k paths found
    if (results.empty()) {
        cout << "No path found from " << source_str << " to " << target_str << ".\n";
    } else {
        for (size_t i = 0; i < results.size(); ++i) {
            cout << "Path " << (i+1) << ": " << results[i] << "\n";
        }
        for (size_t i = results.size(); i < (size_t)k; ++i) {
            cout << "-1\n";
        }
    }

    return 0;
}
