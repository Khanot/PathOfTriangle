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
    int* blossom_mark;   // tableau de marquage pour les bases pendant la contraction
    int blossom_timer;   // compteur pour éviter de réinitialiser blossom_mark
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
/* LCA classique avec marquage temporel */
static int lca(EdmondsCtx* ctx, int a, int b) {
    ctx->lca_timer++;
    while (1) {
        if (a != -1) {
            a = ctx->base[a];
            if (ctx->lca_mark[a] == ctx->lca_timer) return a;
            ctx->lca_mark[a] = ctx->lca_timer;
            a = ctx->match[a] == -1 ? -1 : ctx->p[ctx->match[a]];
        }
        int tmp = a; a = b; b = tmp;
    }
}

/* Marque un chemin du sommet v jusqu'au LCA (b) sans modifier les pères */

/*
static void mark_path(EdmondsCtx* ctx, int v, int b, int children) {
    int iter = 0;
    while (v != -1 && ctx->base[v] != b) {
        int mv = ctx->match[v];
        if (mv == -1) break;
        int next = ctx->p[mv];
        if (next == -1) break;

        // Marquer les bases avec le timer courant
        ctx->blossom_mark[ctx->base[v]] = ctx->blossom_timer;
        ctx->blossom_mark[ctx->base[mv]] = ctx->blossom_timer;

        v = next;
        if (++iter > ctx->n * 2) break;  // sécurité anti-boucle
    }
}
    */
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


            

           
            

            // --- Détection de blossom ---
            if (v == root || (ctx->match[v] != -1 && ctx->p[ctx->match[v]] != -1)) {
                int curbase = lca(ctx, u, v);

                // Allocation d'un tableau pour collecter les sommets du cycle
                int *blossom = malloc(ctx->n * sizeof(int));
                if (!blossom) exit(EXIT_FAILURE);
                int blen = 0;

                // --- Remonter le chemin depuis u jusqu'à curbase ---
                int x = u;
                while (1) {
                    blossom[blen++] = x;               // sommet pair
                    if (ctx->match[x] == -1) break;
                    int mx = ctx->match[x];
                    blossom[blen++] = mx;              // sommet impair
                    if (ctx->base[mx] == curbase) break;
                    x = ctx->p[mx];
                    if (x == -1) break;
                }
                if (blen > 0 && blossom[blen-1] == curbase) blen--;

                int blen_start = blen;

                // --- Remonter le chemin depuis v jusqu'à curbase ---
                x = v;
                while (1) {
                    blossom[blen++] = x;
                    if (ctx->match[x] == -1) break;
                    int mx = ctx->match[x];
                    blossom[blen++] = mx;
                    if (ctx->base[mx] == curbase) break;
                    x = ctx->p[mx];
                    if (x == -1) break;
                }
                if (blen > blen_start && blossom[blen-1] == curbase) blen--;

                // --- Contracter tous les sommets vers curbase ---
                for (int i = 0; i < blen; i++) {
                    ctx->base[blossom[i]] = curbase;
                }
                ctx->base[curbase] = curbase;

                // --- Orienter les parents des anciens impairs ---
                for (int i = 1; i < blen; i += 2) {
                    int odd = blossom[i];
                    int even = blossom[i-1];
                    if (ctx->p[odd] == -1) {
                        ctx->p[odd] = even;
                    }
                }

                // --- Enfiler les nouveaux sommets pairs ---
                for (int i = 0; i < blen; i++) {
                    int node = blossom[i];
                    if (!ctx->used[node]) {
                        ctx->used[node] = 1;
                        ctx->q[tail++] = node;
                    }
                }
                if (!ctx->used[curbase]) {
                    ctx->used[curbase] = 1;
                    ctx->q[tail++] = curbase;
                }

                free(blossom);
            }
            // --- Sommet exposé ---
            else if (ctx->match[v] == -1) {
                
                ctx->p[v] = u;
                return v;
            }
            // --- Progression vers le partenaire ---
            else {
                int w = ctx->match[v];
                
                if (ctx->p[w] == -1) {
                    ctx->p[w] = u;
                    ctx->p[v] = u;   // AJOUT IMPORTANT : parent pour l'impair
                    if (!ctx->used[w]) {
                        ctx->used[w] = 1;
                        ctx->q[tail++] = w;
                        
                    }
                }
            }
        }
    }

    return -1;
}
/* ---------- Augmentation du couplage ---------- */
static void augment(EdmondsCtx* ctx, int v) {
    int iter = 0;
    while (v != -1) {
        //printf("augment iter %d : v = %d\n", iter, v);
        if (v < 0 || v >= ctx->n) {
            //fprintf(stderr, "augment: indice v=%d hors bornes\n", v);
            
        }

        int pv = ctx->p[v];
        //printf("  p[%d] = %d\n", v, pv);
        if (pv < -1 || pv >= ctx->n) {
            //fprintf(stderr, "augment: p[%d] invalide (%d)\n", v, pv);
            
        }
        if (pv == -1) {
            //fprintf(stderr, "augment: p[%d] == -1, impossible de remonter\n", v);
            
        }

        int nv = ctx->match[pv];
        //printf("  match[%d] = %d\n", pv, nv);
        if (nv < -1 || nv >= ctx->n) {
            //fprintf(stderr, "augment: match[%d] invalide (%d)\n", pv, nv);
            
        }

        ctx->match[v] = pv;
        ctx->match[pv] = v;
        //printf("  mariage %d <-> %d\n", v, pv);

        v = nv;
        iter++;
        if (iter > ctx->n * 2) {
            //fprintf(stderr, "augment: trop d'itérations, boucle infinie\n");
            
        }
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


/**
 * Remplace les pointeurs de sommets d’une liste de paires par les sommets
 * de même nom dans le graphe de référence.
 * @param M      liste de paires (modifiée en place)
 * @param ref    graphe de référence (doit contenir les sommets nommés)
 */
void relinkPairs(Pair* M, Graph* ref) {
    for (Pair* p = M; p != NULL; p = p->next) {
        for (int i = 0; i < 2; i++) {
            Vertex* v = getVertex(ref, p->v[i]->name);
            if (v) p->v[i] = v;   // normalement toujours trouvé
        }
    }
}

/**
 * Remplace les pointeurs de sommets d’une liste de sommets par les sommets
 * de même nom dans le graphe de référence.
 */
void relinkVertexCells(VertexCell* S, Graph* ref) {
    for (VertexCell* c = S; c != NULL; c = c->next) {
        Vertex* v = getVertex(ref, c->v->name);
        if (v) c->v = v;
    }
}