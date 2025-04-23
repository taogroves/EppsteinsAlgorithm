#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define INF 1000000000

typedef struct Edge {
    int w;
    int v;
    struct Edge* next;
} Edge;

typedef struct EHeap {
    int rank;
    int key;
    int origin;
    int value;
    struct EHeap* left;
    struct EHeap* right;
} EHeap;

/* Helper function to create a new edge */
Edge* new_edge(int w, int v, Edge* next) {
    Edge* e = (Edge*) malloc(sizeof(Edge));
    e->w = w;
    e->v = v;
    e->next = next;
    return e;
}

/* Leftist heap construction */
EHeap* new_EHeap(int rank, int key, int origin, int value, EHeap* left, EHeap* right) {
    EHeap* h = (EHeap*) malloc(sizeof(EHeap));
    h->rank = rank;
    h->key = key;
    h->origin = origin;
    h->value = value;
    h->left = left;
    h->right = right;
    return h;
}

/* Insert into leftist heap */
EHeap* EHeap_insert(EHeap* a, int k, int u, int v) {
    if (!a || k < a->key) {
        return new_EHeap(1, k, u, v, a, NULL);
    }
    EHeap* r = EHeap_insert(a->right, k, u, v);
    EHeap* l = a->left;
    if (!l || (r && r->rank > l->rank)) {
        a->left = r;
        a->right = l;
    } else {
        a->right = r;
    }
    a->rank = (a->right ? a->right->rank : 0) + 1;
    return a;
}

/* Dijkstra routine on reversed graph */
void dijkstra(Edge** revg, int n, int src, int* d, int* p) {
    int* visited = (int*) calloc(n, sizeof(int));
    int i;
    for (i = 0; i < n; i++) {
        d[i] = INF;
        p[i] = -1;
        visited[i] = 0;
    }
    d[src] = 0;

    /* Min-heap-based approach */
    #define PQSIZE (200000)
    int pqsize = 0;
    int* heapd = (int*) malloc(sizeof(int) * PQSIZE);
    int* heapv = (int*) malloc(sizeof(int) * PQSIZE);

    /* A simple priority queue push/pop (binary heap) */
    void push(int dist, int vtx) {
        if (pqsize >= PQSIZE) return;
        heapd[pqsize] = dist;
        heapv[pqsize] = vtx;
        int curr = pqsize++;
        while (curr > 0) {
            int parent = (curr - 1) / 2;
            if (heapd[parent] <= heapd[curr]) break;
            int tmpd = heapd[curr], tmpv = heapv[curr];
            heapd[curr] = heapd[parent]; heapv[curr] = heapv[parent];
            heapd[parent] = tmpd; heapv[parent] = tmpv;
            curr = parent;
        }
    }

    int popv() {
        if (pqsize == 0) return -1;
        int mnv = heapv[0];
        heapd[0] = heapd[--pqsize];
        heapv[0] = heapv[pqsize];
        int curr = 0;
        while (1) {
            int l = curr * 2 + 1, r = curr * 2 + 2, best = curr;
            if (l < pqsize && heapd[l] < heapd[best]) best = l;
            if (r < pqsize && heapd[r] < heapd[best]) best = r;
            if (best == curr) break;
            int tmpd = heapd[curr], tmpv = heapv[curr];
            heapd[curr] = heapd[best]; heapv[curr] = heapv[best];
            heapd[best] = tmpd; heapv[best] = tmpv;
            curr = best;
        }
        return mnv;
    }

    push(0, src);
    while (pqsize > 0) {
        int u = popv();
        if (u < 0) break;
        if (visited[u]) continue;
        visited[u] = 1;
        Edge* e = revg[u];
        while (e) {
            int nd = d[u] + e->w;
            if (nd < d[e->v]) {
                d[e->v] = nd;
                p[e->v] = u;
                push(nd, e->v);
            }
            e = e->next;
        }
    }
    free(visited);
    free(heapd);
    free(heapv);
}

/* Shortest paths without same arrival logic */
typedef struct {
    int dist;
    int* path; /* We'll just store distance here for printing sum in main, path usage is optional */
} PathItem;

/* Function similar to shortest_paths_no_same_arrival in Python */
PathItem* shortest_paths_no_same_arrival(Edge** g, int n, int src, int dst, int k, int* outSize) {
    /* Build reversed graph */
    Edge** revg = (Edge**) calloc(n, sizeof(Edge*));
    int u;
    for (u = 0; u < n; u++) {
        Edge* e = g[u];
        while (e) {
            Edge* tmp = new_edge(e->w, u, revg[e->v]);
            revg[e->v] = tmp;
            e = e->next;
        }
    }

    /* dijkstra on reversed graph from dst */
    int* d = (int*) malloc(sizeof(int) * n);
    int* p = (int*) malloc(sizeof(int) * n);
    dijkstra(revg, n, dst, d, p);

    if (d[src] == INF) {
        *outSize = 0;
        free(revg);
        free(d);
        free(p);
        return NULL;
    }

    /* build tree t from p */
    Edge** t = (Edge**) calloc(n, sizeof(Edge*));
    for (u = 0; u < n; u++) {
        if (p[u] != -1) {
            t[p[u]] = new_edge(0, u, t[p[u]]);
        }
    }

    /* prepare heaps */
    EHeap** h = (EHeap**) calloc(n, sizeof(EHeap*));
    int* queue = (int*) malloc(sizeof(int) * n);
    int front = 0, back = 0;
    queue[back++] = dst;

    while (front < back) {
        int curr = queue[front++];
        int seenp = 0;
        /* insert edges into heap */
        Edge* ge = g[curr];
        while (ge) {
            if (d[ge->v] != INF) {
                int c = ge->w + d[ge->v] - d[curr];
                if (!seenp && p[curr] == ge->v && c == 0) {
                    seenp = 1;
                } else {
                    h[curr] = EHeap_insert(h[curr], c, curr, ge->v);
                }
            }
            ge = ge->next;
        }
        /* propagate to children in t */
        Edge* te = t[curr];
        while (te) {
            h[te->v] = h[curr];
            queue[back++] = te->v;
            te = te->next;
        }
    }

    /* collect paths (dist only) */
    PathItem* ans = (PathItem*) malloc(sizeof(PathItem) * k);
    int count = 0;
    ans[count].dist = d[src];
    ans[count].path = NULL;
    count++;

    /* if no heap at src, we're done */
    if (!h[src]) {
        *outSize = count; 
        free(revg);
        free(d);
        free(p);
        free(t);
        free(h);
        free(queue);
        return ans;
    }

    /* min-heap for expansions */
    #define PQ2SIZE 200000
    EHeap** heap = (EHeap**) malloc(sizeof(EHeap*) * PQ2SIZE);
    int* cost = (int*) malloc(sizeof(int) * PQ2SIZE);
    int pq2size = 0;
    void push2(EHeap* x, int cst) {
        if (pq2size >= PQ2SIZE) return;
        int idx = pq2size++;
        cost[idx] = cst;
        heap[idx] = x;
        while (idx > 0) {
            int parent = (idx - 1)/2;
            if (cost[parent] <= cost[idx]) break;
            int tmpc = cost[parent]; cost[parent] = cost[idx]; cost[idx] = tmpc;
            EHeap* tmph = heap[parent]; heap[parent] = heap[idx]; heap[idx] = tmph;
            idx = parent;
        }
    }
    EHeap* pop2(int* outCst) {
        if (pq2size == 0) {
            *outCst = -1;
            return NULL;
        }
        EHeap* ret = heap[0];
        *outCst = cost[0];
        cost[0] = cost[--pq2size];
        heap[0] = heap[pq2size];
        int curr = 0;
        while (1) {
            int l = curr*2+1, r = curr*2+2, best = curr;
            if (l < pq2size && cost[l] < cost[best]) best = l;
            if (r < pq2size && cost[r] < cost[best]) best = r;
            if (best == curr) break;
            int tmpc = cost[curr]; cost[curr] = cost[best]; cost[best] = tmpc;
            EHeap* tmph = heap[curr]; heap[curr] = heap[best]; heap[best] = tmph;
            curr = best;
        }
        return ret;
    }

    push2(h[src], d[src] + h[src]->key);

    while (pq2size > 0 && count < k) {
        int cd = 0;
        EHeap* ch = pop2(&cd);
        if (!ch) break;

        int distVal = cd;
        /* if distVal > last appended dist, append a new path entry */
        if (distVal > ans[count - 1].dist) {
            ans[count].dist = distVal;
            ans[count].path = NULL; 
            count++;
        }

        /* expansions: heap at ch->value, left, right */
        if (h[ch->value]) {
            push2(h[ch->value], distVal + h[ch->value]->key);
        }
        if (ch->left) {
            push2(ch->left, distVal + ch->left->key - ch->key);
        }
        if (ch->right) {
            push2(ch->right, distVal + ch->right->key - ch->key);
        }
    }

    *outSize = count;

    free(revg);
    free(d);
    free(p);
    free(t);
    free(h);
    free(queue);
    free(heap);
    free(cost);
    return ans;
}

int main(void) {
    int n, m, s, t, k;
    scanf("%d %d %d %d %d", &n, &m, &s, &t, &k);

    /* Build adjacency list g */
    Edge** g = (Edge**) calloc(n, sizeof(Edge*));
    for (int i = 0; i < m; i++) {
        int u, v, w;
        scanf("%d %d %d", &u, &v, &w);
        g[u] = new_edge(w, v, g[u]);
    }

    int outSize = 0;
    PathItem* paths = shortest_paths_no_same_arrival(g, n, s, t, k, &outSize);
    long long sum_dist = 0;
    for (int i = 0; i < outSize; i++) {
        sum_dist += paths[i].dist;
    }
    printf("%lld\n", sum_dist);

    free(paths);
    for (int i = 0; i < n; i++) {
        Edge* e = g[i];
        while (e) {
            Edge* temp = e->next;
            free(e);
            e = temp;
        }
    }
    free(g);
    return 0;
}