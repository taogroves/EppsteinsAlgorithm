#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;

const long long INF = 1e18;

struct EHeap {
    int rank, key, value;
    EHeap *left, *right;

    EHeap(int r, int k, int v, EHeap* l, EHeap* rgt)
        : rank(r), key(k), value(v), left(l), right(rgt) {}
};

EHeap* insert(EHeap* a, int k, int v) {
    if (!a || k < a->key)
        return new EHeap(1, k, v, a, nullptr);

    EHeap* l = a->left;
    EHeap* r = insert(a->right, k, v);

    if (!l || (r && r->rank > l->rank))
        swap(l, r);

    int newRank = r ? r->rank + 1 : 1;
    return new EHeap(newRank, a->key, a->value, l, r);
}

vector<long long> dijkstra(const vector<vector<pair<int, int>>>& g, int src, vector<int>& parent) {
    int n = g.size();
    vector<long long> dist(n, INF);
    dist[src] = 0;
    parent.assign(n, -1);

    priority_queue<pair<long long, int>, vector<pair<long long, int>>, greater<>> pq;
    pq.emplace(0, src);

    while (!pq.empty()) {
        auto [d_u, u] = pq.top(); pq.pop();
        if (d_u != dist[u]) continue;

        for (auto [w, v] : g[u]) {
            if (d_u + w < dist[v]) {
                dist[v] = d_u + w;
                parent[v] = u;
                pq.emplace(dist[v], v);
            }
        }
    }

    return dist;
}

vector<long long> eppstein_k_shortest_paths(const vector<vector<pair<int, int>>>& g, int src, int dst, int k) {
    int n = g.size();
    vector<vector<pair<int, int>>> revg(n);

    for (int u = 0; u < n; ++u)
        for (auto [w, v] : g[u])
            revg[v].emplace_back(w, u);

    vector<int> parent;
    auto d = dijkstra(revg, dst, parent);

    if (d[src] == INF) return {};

    vector<vector<int>> tree(n);
    for (int u = 0; u < n; ++u)
        if (parent[u] != -1)
            tree[parent[u]].push_back(u);

    vector<EHeap*> h(n, nullptr);
    queue<int> q;
    q.push(dst);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        bool seenParent = false;

        for (auto [w, v] : g[u]) {
            if (d[v] == INF) continue;
            long long costDiff = w + d[v] - d[u];
            if (!seenParent && v == parent[u] && costDiff == 0) {
                seenParent = true;
                continue;
            }
            h[u] = insert(h[u], costDiff, v);
        }

        for (int v : tree[u]) {
            h[v] = h[u];
            q.push(v);
        }
    }

    vector<long long> result = { d[src] };
    if (!h[src]) return result;

    using PQNode = pair<long long, EHeap*>;
    priority_queue<PQNode, vector<PQNode>, greater<>> pq;
    pq.emplace(d[src] + h[src]->key, h[src]);

    while (!pq.empty() && result.size() < k) {
        auto [cost, node] = pq.top(); pq.pop();
        result.push_back(cost);

        if (h[node->value])
            pq.emplace(cost + h[node->value]->key, h[node->value]);
        if (node->left)
            pq.emplace(cost + node->left->key - node->key, node->left);
        if (node->right)
            pq.emplace(cost + node->right->key - node->key, node->right);
    }

    return result;
}

int main() {
    cout << "Enter the number of edges:\n";
    int num_edges;
    cin >> num_edges;

    unordered_map<string, int> node_map;
    unordered_map<int, string> reverse_map;
    vector<tuple<int, int, int>> edges_raw;
    int next_id = 0;

    cout << "Enter each edge in the format 'from to weight' (e.g., A B 3):\n";
    for (int i = 0; i < num_edges; ++i) {
        string u_str, v_str;
        int w;
        cin >> u_str >> v_str >> w;

        if (node_map.count(u_str) == 0) {
            node_map[u_str] = next_id;
            reverse_map[next_id] = u_str;
            ++next_id;
        }
        if (node_map.count(v_str) == 0) {
            node_map[v_str] = next_id;
            reverse_map[next_id] = v_str;
            ++next_id;
        }

        int u = node_map[u_str], v = node_map[v_str];
        edges_raw.emplace_back(u, v, w);
    }

    int n = next_id;
    vector<vector<pair<int, int>>> g(n);
    for (auto [u, v, w] : edges_raw)
        g[u].emplace_back(w, v);

    string source_str, target_str;
    int k;
    cout << "Enter the source node: ";
    cin >> source_str;
    cout << "Enter the target node: ";
    cin >> target_str;
    cout << "Enter the number of paths (k): ";
    cin >> k;

    if (node_map.count(source_str) == 0 || node_map.count(target_str) == 0) {
        cout << "No path found from " << source_str << " to " << target_str << ".\n";
        return 0;
    }

    int source = node_map[source_str];
    int target = node_map[target_str];
    auto results = eppstein_k_shortest_paths(g, source, target, k);

    if (results.empty()) {
        cout << "No path found from " << source_str << " to " << target_str << ".\n";
    } else {
        for (size_t i = 0; i < results.size(); ++i) {
            cout << "Path " << (i + 1) << ": " << results[i] << "\n";
        }
        for (int i = results.size(); i < k; ++i) {
            cout << "-1\n";
        }
    }

    return 0;
}
