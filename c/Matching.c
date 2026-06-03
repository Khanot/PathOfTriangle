#include <stdlib.h>
#include <string.h>
#include "Matching.h"

/* ---------- Contexte de l'algorithme ---------- */
typedef struct {
    int n;               // nombre de sommets
    int* match;          // couplage courant
    int* p;              // père dans l'arbre alterné
    int* base;           // représentant du blossom
    int* q;              // file pour le BFS
    int* used;           // marqueur BFS
    int* lca_mark;       // marqueur pour le LCA
    int  lca_timer;
} EdmondsCtx;

/* ---------- Allocation / libération ---------- */
static void edmonds_init(EdmondsCtx* ctx, int n) {
    ctx->n = n;
    ctx->match = malloc(n * sizeof(int));
    ctx->p     = malloc(n * sizeof(int));
    ctx->base  = malloc(n * sizeof(int));
    ctx->q     = malloc(n * sizeof(int));
    ctx->used  = malloc(n * sizeof(int));
    ctx->lca_mark = malloc(n * sizeof(int));
    ctx->lca_timer = 0;
}

static void edmonds_free(EdmondsCtx* ctx) {
    free(ctx->match);
    free(ctx->p);
    free(ctx->base);
    free(ctx->q);
    free(ctx->used);
    free(ctx->lca_mark);
}

/* ---------- LCA ---------- */
static int lca(EdmondsCtx* ctx, int a, int b) {
    ctx->lca_timer++;
    while (1) {
        if (a != -1) {
            a = ctx->base[a];
            if (ctx->lca_mark[a] == ctx->lca_timer) return a;
            ctx->lca_mark[a] = ctx->lca_timer;
            a = (ctx->match[a] != -1) ? ctx->p[ctx->match[a]] : -1;
        }
        int tmp = a; a = b; b = tmp;
    }
}

/* ---------- Marquage d'un chemin vers le LCA ---------- */
static void mark_path(EdmondsCtx* ctx, int v, int b, int children) {
    while (ctx->base[v] != b) {
        int mv = ctx->match[v];
        ctx->used[ctx->base[v]] = children;   // réutilise temporairement `used` pour le blossom
        ctx->used[ctx->base[mv]] = children;
        ctx->p[v] = mv;
        v = ctx->p[mv];
    }
}

/* ---------- Recherche d'un chemin augmentant ---------- */
static int find_path(EdmondsCtx* ctx, Graph* g, int root) {
    int n = ctx->n;
    memset(ctx->used, 0, n * sizeof(int));
    memset(ctx->p, -1, n * sizeof(int));
    for (int i = 0; i < n; i++) ctx->base[i] = i;

    int head = 0, tail = 0;
    ctx->used[root] = 1;
    ctx->q[tail++] = root;

    while (head < tail) {
        int u = ctx->q[head++];
        AdjList* adj = &g->adj[u];
        for (int i = 0; i < adj->size; i++) {
            Edge* e = adj->edges[i];
            Vertex* vp = (e->endpoints[0] == g->vertices[u]) ? e->endpoints[1] : e->endpoints[0];
            int v = vp->id;

            if (ctx->base[u] != ctx->base[v] && ctx->match[u] != v) {
                if (v == root || (ctx->match[v] != -1 && ctx->p[ctx->match[v]] != -1)) {
                    // blossom détecté
                    int curbase = lca(ctx, u, v);
                    memset(ctx->used, 0, n * sizeof(int));   // on efface les marquages précédents
                    mark_path(ctx, u, curbase, 1);
                    mark_path(ctx, v, curbase, 2);

                    for (int i = 0; i < n; i++) {
                        if (ctx->used[ctx->base[i]] == 1 || ctx->used[ctx->base[i]] == 2) {
                            ctx->base[i] = curbase;
                            if (!ctx->used[i]) {
                                ctx->used[i] = 1;   // marque comme visité pour le BFS
                                ctx->q[tail++] = i;
                            }
                        }
                    }
                } else if (ctx->p[v] == -1) {
                    ctx->p[v] = u;
                    if (ctx->match[v] == -1)
                        return v;   // sommet exposé atteint
                    ctx->used[ctx->match[v]] = 1;
                    ctx->q[tail++] = ctx->match[v];
                }
            }
        }
    }
    return -1;
}

/* ---------- Augmentation du couplage ---------- */
static void augment(EdmondsCtx* ctx, int v) {
    while (v != -1) {
        int pv = ctx->p[v];
        int nv = ctx->match[pv];
        ctx->match[v] = pv;
        ctx->match[pv] = v;
        v = nv;
    }
}

/* ---------- Fonction principale ---------- */
int maximumMatching(Graph* g, int* out_match) {
    int n = g->nbv;
    EdmondsCtx ctx;
    edmonds_init(&ctx, n);
    for (int i = 0; i < n; i++) ctx.match[i] = -1;

    int res = 0;
    for (int i = 0; i < n; i++) {
        if (ctx.match[i] == -1) {
            int v = find_path(&ctx, g, i);
            if (v != -1) {
                augment(&ctx, v);
                res++;
            }
        }
    }

    memcpy(out_match, ctx.match, n * sizeof(int));
    edmonds_free(&ctx);
    return res;
}


int maximumMatchingBad(Graph* g, int* out_match) {
    int n = g->nbv;
    int m = g->nbe;

    // 1. Récupérer les arêtes sous forme de couples d’indices
    int* u = malloc(m * sizeof(int));
    int* v = malloc(m * sizeof(int));
    if (!u || !v) { free(u); free(v); return 0; }
    for (int i = 0; i < m; i++) {
        Edge* e = g->edges[i];
        u[i] = e->endpoints[0]->id;
        v[i] = e->endpoints[1]->id;
    }

    // 2. États pour le backtracking itératif
    // state[i] = 0 : arête i non encore traitée
    // state[i] = 1 : branche "ne pas prendre" déjà essayée
    // state[i] = 2 : branche "prendre" déjà essayée (ou impossible)
    int* state = calloc(m, sizeof(int));
    int* cur_match = malloc(n * sizeof(int));
    int* best_match = malloc(n * sizeof(int));
    if (!state || !cur_match || !best_match) {
        free(u); free(v); free(state); free(cur_match); free(best_match);
        return 0;
    }
    for (int i = 0; i < n; i++) cur_match[i] = best_match[i] = -1;

    int cur_size = 0, best_size = 0;
    int pos = 0;   // indice de l’arête courante

    // Boucle principale du backtracking
    while (pos >= 0) {
        if (pos == m) {
            // Toutes les arêtes ont été traitées → on met à jour le meilleur
            if (cur_size > best_size) {
                best_size = cur_size;
                memcpy(best_match, cur_match, n * sizeof(int));
            }
            pos--;   // revenir à l’arête précédente
            continue;
        }

        int edge_idx = pos;

        if (state[edge_idx] == 0) {
            // Essayer d’abord de ne PAS prendre l’arête
            state[edge_idx] = 1;   // on a choisi "ne pas prendre"
            pos++;                 // passer à l’arête suivante
        }
        else if (state[edge_idx] == 1) {
            // Essayer maintenant de PRENDRE l’arête (si possible)
            int nu = u[edge_idx];
            int nv = v[edge_idx];
            if (cur_match[nu] == -1 && cur_match[nv] == -1) {
                // On peut prendre l’arête
                cur_match[nu] = nv;
                cur_match[nv] = nu;
                cur_size++;
                state[edge_idx] = 2;   // marquer qu’on a essayé la prise
                pos++;
            } else {
                // Impossible de prendre → on a fini cette arête, on revient en arrière
                state[edge_idx] = 2;   // pour éviter de rester bloqué
                pos--;
            }
        }
        else {
            // state == 2 : on a déjà exploré les deux branches (ou l’arête était impossible)
            // Il faut restaurer l’état si on l’avait prise
            if (cur_match[u[edge_idx]] == v[edge_idx] && cur_match[v[edge_idx]] == u[edge_idx]) {
                cur_match[u[edge_idx]] = -1;
                cur_match[v[edge_idx]] = -1;
                cur_size--;
            }
            state[edge_idx] = 0;   // réinitialiser pour laisser propre
            pos--;
        }
    }

    // 3. Copier le meilleur couplage dans le tableau de sortie
    memcpy(out_match, best_match, n * sizeof(int));

    // Nettoyage
    free(u); free(v); free(state);
    free(cur_match); free(best_match);
    return best_size;
}