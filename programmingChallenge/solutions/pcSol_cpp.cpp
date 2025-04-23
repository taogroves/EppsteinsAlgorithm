#include <iostream>
#include <vector>
using namespace std;

static const long long INF = (long long)1e18;

/* Dijkstra on adjacency list (for reversed graph here) */
pair<vector<long long>, vector<int>> dijkstra(const vector<vector<pair<long long,int>>> &g, int src) {
    int n = (int)g.size();
    vector<long long> d(n, INF);
    vector<int> p(n, -1);
    d[src] = 0;

    priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<pair<long long,int>>> pq;
    pq.push({0, src});

    while(!pq.empty()){
        auto [distU, u] = pq.top();
        pq.pop();
        if(distU != d[u]) continue;
        for(auto &edge : g[u]){
            long long w = edge.first;
            int v = edge.second;
            if(d[u] + w < d[v]){
                d[v] = d[u] + w;
                p[v] = u;
                pq.push({d[v], v});
            }
        }
    }
    return {d, p};
}

/* Leftist heap structure */
struct EHeap {
    long long rank;
    long long key;
    int origin;
    int value;
    EHeap* left;
    EHeap* right;

    EHeap(long long r, long long k, int o, int v, EHeap* l, EHeap* rr)
        : rank(r), key(k), origin(o), value(v), left(l), right(rr) {}

    static EHeap* insertNode(EHeap* a, long long k, int u, int v) {
        if(!a || k < a->key) {
            return new EHeap(1, k, u, v, a, nullptr);
        }
        a->right = insertNode(a->right, k, u, v);
        if(!a->left || (a->right && a->right->rank > a->left->rank)) {
            swap(a->left, a->right);
        }
        a->rank = (a->right ? a->right->rank : 0) + 1;
        return a;
    }
};

/* shortest_paths_no_same_arrival, returns vector of (distance, path) */
vector<pair<long long, vector<pair<int,int>>>> 
shortest_paths_no_same_arrival(const vector<vector<pair<long long,int>>> &g,
                               int src, int dst, int k) {
    int n = (int)g.size();
    // Build reversed graph
    vector<vector<pair<long long,int>>> revg(n);
    for(int u=0; u<n; u++){
        for(auto &edge : g[u]){
            long long w = edge.first;
            int v = edge.second;
            revg[v].push_back({w, u});
        }
    }

    // Dijkstra on reversed graph from dst
    auto [d, p] = dijkstra(revg, dst);
    if(d[src] == INF) {
        return {};
    }

    // Build tree t from p
    vector<vector<int>> t(n);
    for(int u=0; u<n; u++){
        if(p[u] != -1){
            t[p[u]].push_back(u);
        }
    }

    // Prepare heaps
    vector<EHeap*> h(n, nullptr);
    queue<int> bfsq;
    bfsq.push(dst);

    while(!bfsq.empty()){
        int u = bfsq.front(); 
        bfsq.pop();
        bool seenp = false;
        // Insert edges from g[u]
        for(auto &edge : g[u]){
            long long w = edge.first;
            int v = edge.second;
            if(d[v] == INF) continue;
            long long c = w + d[v] - d[u];
            if(!seenp && v == p[u] && c == 0){
                seenp = true;
                continue;
            }
            h[u] = EHeap::insertNode(h[u], c, u, v);
        }
        // Propagate to children in t
        for(int child : t[u]){
            h[child] = h[u];
            bfsq.push(child);
        }
    }

    // ans stores (distance, path)
    vector<pair<long long, vector<pair<int,int>>>> ans;
    ans.push_back({d[src], {}});

    // If no heap at src, we only have the shortest path distance
    if(!h[src]) {
        return ans;
    }

    // Priority queue for expansions: (costSoFar, EHeap*, currentPath)
    typedef tuple<long long, EHeap*, vector<pair<int,int>>> State;
    priority_queue<State, vector<State>, greater<State>> pq;
    pq.push({d[src] + h[src]->key, h[src], {}});

    while(!pq.empty() && (int)ans.size() < k){
        auto [cd, ch, path] = pq.top();
        pq.pop();
        auto new_path = path;
        new_path.push_back({ch->origin, ch->value});

        // If cost is strictly larger, add to ans
        if(cd > ans.back().first){
            ans.push_back({cd, new_path});
        }

        // Expand
        if(h[ch->value]) {
            pq.push({cd + h[ch->value]->key, h[ch->value], new_path});
        }
        if(ch->left) {
            pq.push({cd + ch->left->key - ch->key, ch->left, path});
        }
        if(ch->right) {
            pq.push({cd + ch->right->key - ch->key, ch->right, path});
        }
    }
    return ans;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, s, t, k;
    cin >> n >> m >> s >> t >> k;

    vector<vector<pair<long long,int>>> graph(n);
    for(int i=0; i<m; i++){
        long long w;
        int u, v;
        cin >> u >> v >> w;
        graph[u].push_back({w, v});
    }

    // Compute up to k distinct paths with no same arrival
    auto paths = shortest_paths_no_same_arrival(graph, s, t, k);

    // Output the length of the last (longest) path
    cout << paths.back().first << '\n';
    
    return 0;
}