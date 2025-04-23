#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define INF 1000000000000000000LL

typedef struct {
    long long weight;
    int to;
} Edge;

typedef struct {
    Edge* edges;
    int size;
    int capacity;
} EdgeList;

typedef struct {
    long long dist;
    int node;
} PQNode;

typedef struct {
    PQNode* data;
    int size;
    int capacity;
} MinHeap;

typedef struct EHeap {
    long long rank;
    long long key;
    int origin;
    int value;
    struct EHeap* left;
    struct EHeap* right;
} EHeap;

typedef struct {
    long long cost;
    EHeap* node;
} ExpandNode;

typedef struct {
    ExpandNode* data;
    int size;
    int capacity;
} ExpandPQ;

/* --- Utility functions --- */

void init_edge_list(EdgeList* list) {
    list->size = 0;
    list->capacity = 4;
    list->edges = (Edge*)malloc(sizeof(Edge) * list->capacity);
}

void add_edge(EdgeList* list, long long weight, int to) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        list->edges = (Edge*)realloc(list->edges, sizeof(Edge) * list->capacity);
    }
    list->edges[list->size++] = (Edge){weight, to};
}

/* --- Dijkstra heap --- */

MinHeap* create_heap(int capacity) {
    MinHeap* h = (MinHeap*)malloc(sizeof(MinHeap));
    h->data = (PQNode*)malloc(sizeof(PQNode) * capacity);
    h->size = 0;
    h->capacity = capacity;
    return h;
}

void push_heap(MinHeap* h, long long dist, int node) {
    int i = h->size++;
    h->data[i] = (PQNode){dist, node};
    while (i && h->data[(i - 1) / 2].dist > h->data[i].dist) {
        PQNode tmp = h->data[i];
        h->data[i] = h->data[(i - 1) / 2];
        h->data[(i - 1) / 2] = tmp;
        i = (i - 1) / 2;
    }
}

PQNode pop_heap(MinHeap* h) {
    PQNode top = h->data[0];
    h->data[0] = h->data[--h->size];
    int i = 0;
    while (1) {
        int l = 2*i + 1, r = 2*i + 2, smallest = i;
        if (l < h->size && h->data[l].dist < h->data[smallest].dist) smallest = l;
        if (r < h->size && h->data[r].dist < h->data[smallest].dist) smallest = r;
        if (smallest == i) break;
        PQNode tmp = h->data[i];
        h->data[i] = h->data[smallest];
        h->data[smallest] = tmp;
        i = smallest;
    }
    return top;
}

int heap_empty(MinHeap* h) {
    return h->size == 0;
}

/* --- Leftist heap --- */

EHeap* insert_node(EHeap* a, long long k, int u, int v) {
    EHeap* node = (EHeap*)malloc(sizeof(EHeap));
    node->rank = 1;
    node->key = k;
    node->origin = u;
    node->value = v;
    node->left = a;
    node->right = NULL;

    if (!a || k < a->key) return node;

    a->right = insert_node(a->right, k, u, v);
    if (!a->left || (a->right && a->right->rank > a->left->rank)) {
        EHeap* tmp = a->left;
        a->left = a->right;
        a->right = tmp;
    }
    a->rank = (a->right ? a->right->rank : 0) + 1;
    return a;
}

/* --- Expansion queue --- */

ExpandPQ* create_expand_pq(int capacity) {
    ExpandPQ* pq = (ExpandPQ*)malloc(sizeof(ExpandPQ));
    pq->data = (ExpandNode*)malloc(sizeof(ExpandNode) * capacity);
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

void push_expand(ExpandPQ* pq, long long cost, EHeap* node) {
    int i = pq->size++;
    pq->data[i] = (ExpandNode){cost, node};
    while (i && pq->data[(i - 1) / 2].cost > pq->data[i].cost) {
        ExpandNode tmp = pq->data[i];
        pq->data[i] = pq->data[(i - 1) / 2];
        pq->data[(i - 1) / 2] = tmp;
        i = (i - 1) / 2;
    }
}

ExpandNode pop_expand(ExpandPQ* pq) {
    ExpandNode top = pq->data[0];
    pq->data[0] = pq->data[--pq->size];
    int i = 0;
    while (1) {
        int l = 2*i + 1, r = 2*i + 2, smallest = i;
        if (l < pq->size && pq->data[l].cost < pq->data[smallest].cost) smallest = l;
        if (r < pq->size && pq->data[r].cost < pq->data[smallest].cost) smallest = r;
        if (smallest == i) break;
        ExpandNode tmp = pq->data[i];
        pq->data[i] = pq->data[smallest];
        pq->data[smallest] = tmp;
        i = smallest;
    }
    return top;
}

/* --- Dijkstra from reversed graph --- */

void dijkstra(EdgeList* graph, int n, int src, long long* dist, int* pred) {
    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        pred[i] = -1;
    }
    dist[src] = 0;

    MinHeap* pq = create_heap(n * 2);
    push_heap(pq, 0, src);

    while (!heap_empty(pq)) {
        PQNode top = pop_heap(pq);
        int u = top.node;
        if (top.dist != dist[u]) continue;

        for (int i = 0; i < graph[u].size; i++) {
            int v = graph[u].edges[i].to;
            long long w = graph[u].edges[i].weight;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pred[v] = u;
                push_heap(pq, dist[v], v);
            }
        }
    }

    free(pq->data);
    free(pq);
}

/* --- Main logic --- */

int main() {
    int n, m, s, t, k;
    scanf("%d %d %d %d %d", &n, &m, &s, &t, &k);

    EdgeList* graph = (EdgeList*)malloc(sizeof(EdgeList) * n);
    EdgeList* revg = (EdgeList*)malloc(sizeof(EdgeList) * n);
    for (int i = 0; i < n; i++) {
        init_edge_list(&graph[i]);
        init_edge_list(&revg[i]);
    }

    for (int i = 0; i < m; i++) {
        int u, v;
        long long w;
        scanf("%d %d %lld", &u, &v, &w);
        add_edge(&graph[u], w, v);
        add_edge(&revg[v], w, u);
    }

    long long* d = (long long*)malloc(sizeof(long long) * n);
    int* p = (int*)malloc(sizeof(int) * n);
    dijkstra(revg, n, t, d, p);

    if (d[s] == INF) {
        printf("-1\n");
        return 0;
    }

    // Build tree from p
    EdgeList* tree = (EdgeList*)malloc(sizeof(EdgeList) * n);
    for (int i = 0; i < n; i++) init_edge_list(&tree[i]);
    for (int i = 0; i < n; i++) {
        if (p[i] != -1) {
            add_edge(&tree[p[i]], 0, i);
        }
    }

    EHeap** h = (EHeap**)calloc(n, sizeof(EHeap*));
    int* queue = (int*)malloc(sizeof(int) * n);
    int qh = 0, qt = 0;
    queue[qt++] = t;

    while (qh < qt) {
        int u = queue[qh++];
        int seenp = 0;
        for (int i = 0; i < graph[u].size; i++) {
            int v = graph[u].edges[i].to;
            long long w = graph[u].edges[i].weight;
            if (d[v] == INF) continue;
            long long c = w + d[v] - d[u];
            if (!seenp && v == p[u] && c == 0) {
                seenp = 1;
                continue;
            }
            h[u] = insert_node(h[u], c, u, v);
        }

        for (int i = 0; i < tree[u].size; i++) {
            int child = tree[u].edges[i].to;
            h[child] = h[u];
            queue[qt++] = child;
        }
    }

    long long last_dist = d[s];
    if (!h[s]) {
        printf("%lld\n", last_dist);
        return 0;
    }

    ExpandPQ* pq = create_expand_pq(k * 10);
    push_expand(pq, d[s] + h[s]->key, h[s]);

    int found = 1;
    while (found < k && pq->size > 0) {
        ExpandNode top = pop_expand(pq);
        long long cost = top.cost;
        EHeap* ch = top.node;

        if (cost > last_dist) {
            last_dist = cost;
            found++;
        }

        if (h[ch->value]) push_expand(pq, cost + h[ch->value]->key, h[ch->value]);
        if (ch->left) push_expand(pq, cost + ch->left->key - ch->key, ch->left);
        if (ch->right) push_expand(pq, cost + ch->right->key - ch->key, ch->right);
    }

    printf("%lld\n", last_dist);

    return 0;
}
