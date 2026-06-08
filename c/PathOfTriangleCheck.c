#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "PathOfTriangleCheck.h"

/* ---------- Structures internes (identiques à la version précédente) ---------- */
#define MAX_INDEX 200

typedef struct {
    Vertex** vertices;
    int count;
    int capacity;
} VertexSet;

typedef struct {
    VertexSet path[MAX_INDEX];
    VertexSet X[MAX_INDEX];
    VertexSet hatX[MAX_INDEX];
    VertexSet L[MAX_INDEX];
    VertexSet M[MAX_INDEX];
    VertexSet R[MAX_INDEX];
    int max_index;
    int n;
} PoTStructure;

/* ---------- Fonctions utilitaires (mêmes que précédemment) ---------- */



static bool areAdjacent(Graph* g, Vertex* a, Vertex* b) {
    int id_a = a->id;
    AdjList* adj = &g->adj[id_a];
    for (int i = 0; i < adj->size; i++) {
        Edge* e = adj->edges[i];
        Vertex* other = (e->endpoints[0] == a) ? e->endpoints[1] : e->endpoints[0];
        if (other == b) return true;
    }
    return false;
}

static void addToSet(VertexSet* set, Vertex* v) {
    if (set->count == set->capacity) {
        int newcap = set->capacity == 0 ? 4 : set->capacity * 2;
        set->vertices = realloc(set->vertices, newcap * sizeof(Vertex*));
        set->capacity = newcap;
    }
    set->vertices[set->count++] = v;
}

static void freeSet(VertexSet* set) {
    free(set->vertices);
    set->vertices = NULL;
    set->count = set->capacity = 0;
}

static bool buildPoTStructure(Graph* g, PoTStructure* ps, int verbose) {
    memset(ps, 0, sizeof(PoTStructure));
    ps->max_index = 0;

    for (int i = 0; i < g->nbv; i++) {
        Vertex* v = g->vertices[i];
        char c = v->name[0];
        if (c != 's' && c != 'm' && c != 'l' && c != 'r')
            return false;
        int base = getBaseIndex(v->name);
        
        if (base > ps->max_index) ps->max_index = base;

        addToSet(&ps->X[base], v);

        if (base % 2 == 1) {
            if (c == 'l') addToSet(&ps->L[base], v);
            else if (c == 'm') addToSet(&ps->M[base], v);
            else if (c == 'r') addToSet(&ps->R[base], v);
        }

        if (base % 2 == 0) {
            // Compter le nombre d'underscores dans le nom
            int underscores = 0;
            for (const char* p = v->name; *p; p++) {
                if (*p == '_') underscores++;
            }
            // Inclure dans hatX si au plus un underscore (donc pas un négatif)
            if (underscores <= 1) {
                addToSet(&ps->hatX[base], v);
            }
            if (underscores == 0) {
                addToSet(&ps->path[base], v);
            }
        }
    }
    if (verbose){
        printf("=== Structure PoT ===\n");
        printf("max_index = %d, n = %d\n", ps->max_index, ps->n);
        for (int i = 1; i <= ps->max_index; i++) {
            printf("X_%d : ", i);
            for (int k = 0; k < ps->X[i].count; k++) {
                printf("%s ", ps->X[i].vertices[k]->name);
            }
            printf("\n");
            if (i % 2 == 0) {
                printf("  hatX_%d : ", i);
                for (int k = 0; k < ps->hatX[i].count; k++) {
                    printf("%s ", ps->hatX[i].vertices[k]->name);
                }
                printf("\n");
            } else {
                printf("  L_%d : ", i);
                for (int k = 0; k < ps->L[i].count; k++) printf("%s ", ps->L[i].vertices[k]->name);
                printf("\n  M_%d : ", i);
                for (int k = 0; k < ps->M[i].count; k++) printf("%s ", ps->M[i].vertices[k]->name);
                printf("\n  R_%d : ", i);
                for (int k = 0; k < ps->R[i].count; k++) printf("%s ", ps->R[i].vertices[k]->name);
                printf("\n");
            }
        }
    } 

    
    ps->n = floor((ps->max_index ) / 2);
    
    

    

    return true;
}

static void freePoTStructure(PoTStructure* ps) {
    for (int i = 0; i < MAX_INDEX; i++) {
        freeSet(&ps->X[i]);
        freeSet(&ps->hatX[i]);
        freeSet(&ps->L[i]);
        freeSet(&ps->M[i]);
        freeSet(&ps->R[i]);
    }
}

/* ---------- Vérification des propriétés (remplit un tableau de 7 entiers) ---------- */
static int* checkProperties(Graph* g, PoTStructure* ps) {
    int* ok = malloc(7 * sizeof(int));
    if (!ok) return NULL;
    for (int i = 0; i < 7; i++) ok[i] = 1;  // par défaut vrai

    int n = ps->n;

    // P1
    // P1 – existence de hatX, bords de taille 1, pas de deux multipliés consécutifs
    for (int i = 1; i <= n; i++) {
        if (ps->hatX[2*i].count == 0) { ok[0] = 0; break; }
    }
    if (ok[0] && (ps->hatX[2].count != 1 || ps->hatX[2*n].count != 1)){
        ok[0] = 0;
    }
    if (ok[0]) {
        for (int i = 1; i < n; i++) {
            // Chaque colonne paire doit avoir exactement un sommet principal (sans underscore)
            if (ps->path[2*i].count != 1 || ps->path[2*i+2].count != 1) {
                ok[0] = 0;
                break;
            }
            Vertex* vi = ps->path[2*i].vertices[0];
            Vertex* vj = ps->path[2*i+2].vertices[0];
            // Un sommet est multiplié s’il ne porte pas le suffixe « n »
            bool i_mult = (strstr(vi->name, "n") == NULL);
            bool j_mult = (strstr(vj->name, "n") == NULL);
            if (i_mult && j_mult) {   // deux multipliés consécutifs → interdit
                ok[0] = 0;
                break;
            }
        }
    }

    // P2
    for (int i = 1; i <= ps->max_index && ok[1]; i++) {
        for (int j = i+1; j <= ps->max_index && ok[1]; j++) {
            int diff = j - i;
            if (diff == 1) continue;
            if (diff % 3 == 2) {
                for (int a = 0; a < ps->X[i].count && ok[1]; a++) {
                    for (int b = 0; b < ps->X[j].count && ok[1]; b++) {
                        Vertex* u = ps->X[i].vertices[a];
                        Vertex* v = ps->X[j].vertices[b];
                        if (!areAdjacent(g, u, v)) {
                            if (i%2==1 && j%2==1 && j==i+2) continue; // autorisé
                            if (i%2==0 && j%2==0) {
                                bool u_in_hat = false, v_in_hat = false;
                                for (int k=0; k<ps->hatX[i].count; k++)
                                    if (ps->hatX[i].vertices[k]==u) u_in_hat=true;
                                for (int k=0; k<ps->hatX[j].count; k++)
                                    if (ps->hatX[j].vertices[k]==v) v_in_hat=true;
                                if (u_in_hat || v_in_hat) { ok[1] = 0; break; }
                            } else {
                                ok[1] = 0;
                                break;
                            }
                        }
                    }
                }
            } else { // diff % 3 != 2 et diff != 1
                for (int a = 0; a < ps->X[i].count && ok[1]; a++) {
                    for (int b = 0; b < ps->X[j].count && ok[1]; b++) {
                        if (areAdjacent(g, ps->X[i].vertices[a], ps->X[j].vertices[b])) {
                            ok[1] = 0;
                            break;
                        }
                    }
                }
            }
        }
    }

    // P3
    for (int i = 1; i <= n+1; i++) {
        int idx = 2*i - 1;
        if (ps->L[idx].count + ps->M[idx].count + ps->R[idx].count != ps->X[idx].count)
            ok[2] = 0;
        if (i == 1 && (ps->L[idx].count != 0 || ps->M[idx].count != 0))
            ok[2] = 0;
        if (i == n+1 && (ps->M[idx].count != 0 || ps->R[idx].count != 0))
            ok[2] = 0;
    }

    // P4
    if (ps->R[1].count == 0) {
        if (n < 2 || ps->hatX[4].count <= 1) ok[3] = 0;
    }
    if (ps->L[2*n+1].count == 0) {
        if (n < 2 || ps->hatX[2*n-2].count <= 1) ok[3] = 0;
    }

    // P5
    for (int i = 1; i <= n && ok[4]; i++) {
        int idx = 2*i;
        // X_{2i} anti-complet à L_{2i-1} ∪ R_{2i+1}
        for (int a = 0; a < ps->X[idx].count && ok[4]; a++) {
            for (int b = 0; b < ps->L[idx-1].count + ps->R[idx+1].count && ok[4]; b++) {
                Vertex* target = (b < ps->L[idx-1].count) ?
                    ps->L[idx-1].vertices[b] :
                    ps->R[idx+1].vertices[b - ps->L[idx-1].count];
                if (areAdjacent(g, ps->X[idx].vertices[a], target))
                    ok[4] = 0;
            }
        }
        // X_{2i} \ hatX_{2i} anti-complet à M_{2i-1} ∪ M_{2i+1}
        for (int a = 0; a < ps->X[idx].count && ok[4]; a++) {
            Vertex* u = ps->X[idx].vertices[a];
            bool in_hat = false;
            for (int k = 0; k < ps->hatX[idx].count; k++)
                if (ps->hatX[idx].vertices[k] == u) { in_hat = true; break; }
            if (in_hat) continue;
            for (int b = 0; b < ps->M[idx-1].count + ps->M[idx+1].count && ok[4]; b++) {
                Vertex* target = (b < ps->M[idx-1].count) ?
                    ps->M[idx-1].vertices[b] :
                    ps->M[idx+1].vertices[b - ps->M[idx-1].count];
                if (areAdjacent(g, u, target))
                    ok[4] = 0;
            }
        }
        // Condition sur les arêtes R-L
        for (int a = 0; a < ps->R[idx-1].count && ok[4]; a++) {
            for (int b = 0; b < ps->L[idx+1].count && ok[4]; b++) {
                Vertex* r = ps->R[idx-1].vertices[a];
                Vertex* l = ps->L[idx+1].vertices[b];
                if (areAdjacent(g, r, l)) {
                    for (int c = 0; c < ps->X[idx].count && ok[4]; c++) {
                        Vertex* u = ps->X[idx].vertices[c];
                        bool in_hat = false;
                        for (int k = 0; k < ps->hatX[idx].count; k++)
                            if (ps->hatX[idx].vertices[k] == u) { in_hat = true; break; }
                        if (in_hat) continue;
                        bool adj_r = areAdjacent(g, u, r);
                        bool adj_l = areAdjacent(g, u, l);
                        if (adj_r == adj_l) { ok[4] = 0; break; }
                    }
                }
            }
        }
    }

    // P6
    //bool any_hat_one = false;
    for (int i = 1; i <= n; i++) {
        if (ps->hatX[2*i].count != 1) continue;
        //any_hat_one = true;
        int idx = 2*i;
        Vertex* hat_v = ps->hatX[idx].vertices[0];

        // 1. Absence d'arêtes entre M et L/M, R et M
        for (int a = 0; a < ps->M[idx-1].count && ok[5]; a++) {
            for (int b = 0; b < ps->L[idx+1].count + ps->M[idx+1].count && ok[5]; b++) {
                Vertex* v = (b < ps->L[idx+1].count) ? ps->L[idx+1].vertices[b]
                                                      : ps->M[idx+1].vertices[b - ps->L[idx+1].count];
                if (areAdjacent(g, ps->M[idx-1].vertices[a], v)) ok[5] = 1000;
            }
        }
        for (int a = 0; a < ps->R[idx-1].count && ok[5]; a++) {
            for (int b = 0; b < ps->M[idx+1].count && ok[5]; b++) {
                if (areAdjacent(g, ps->R[idx-1].vertices[a], ps->M[idx+1].vertices[b]))
                    ok[5] = 10;
            }
        }
        // Vérifier que R et L sont couplés
        if (ok[5]) {
            for (int a = 0; a < ps->R[idx-1].count; a++) {
                int cnt = 0;
                for (int b = 0; b < ps->L[idx+1].count; b++) {
                    if (areAdjacent(g, ps->R[idx-1].vertices[a], ps->L[idx+1].vertices[b]))
                        cnt++;
                }
                if (cnt != 1) ok[5] = 20;
            }
            for (int b = 0; b < ps->L[idx+1].count; b++) {
                int cnt = 0;
                for (int a = 0; a < ps->R[idx-1].count; a++) {
                    if (areAdjacent(g, ps->R[idx-1].vertices[a], ps->L[idx+1].vertices[b]))
                        cnt++;
                }
                if (cnt != 1) ok[5] = 30;
            }
        }
        // 2. Complet à l'union
        if (ok[5]) {
            for (int k = 0; k < ps->R[idx-1].count + ps->M[idx-1].count + ps->L[idx+1].count + ps->M[idx+1].count; k++) {
                Vertex* target;
                if (k < ps->R[idx-1].count) target = ps->R[idx-1].vertices[k];
                else if (k < ps->R[idx-1].count + ps->M[idx-1].count) target = ps->M[idx-1].vertices[k - ps->R[idx-1].count];
                else if (k < ps->R[idx-1].count + ps->M[idx-1].count + ps->L[idx+1].count) target = ps->L[idx+1].vertices[k - ps->R[idx-1].count - ps->M[idx-1].count];
                else target = ps->M[idx+1].vertices[k - ps->R[idx-1].count - ps->M[idx-1].count - ps->L[idx+1].count];
                if (!areAdjacent(g, hat_v, target)) { ok[5] = 40; break; }
            }
        }
        // 3. Complet entre L et X_{2i+1}, X_{2i-1} et R
        if (i > 1 && ok[5]) {
            for (int a = 0; a < ps->L[idx-1].count; a++) {
                for (int b = 0; b < ps->X[2*i+1].count; b++) {
                    if (!areAdjacent(g, ps->L[idx-1].vertices[a], ps->X[2*i+1].vertices[b])) {
                        ok[5] = 50; break;
                    }
                }
            }
        }
        if (i < n && ok[5]) {
            for (int a = 0; a < ps->X[2*i-1].count; a++) {
                for (int b = 0; b < ps->R[idx+1].count; b++) {
                    if (!areAdjacent(g, ps->X[2*i-1].vertices[a], ps->R[idx+1].vertices[b])) {
                        ok[5] = 60; break;
                    }
                }
            }
        }
        // 4. Couplage entre M et hatX
        if (i > 1 && ok[5]) {
            for (int a = 0; a < ps->M[idx-1].count; a++) {
                int cnt = 0;
                for (int b = 0; b < ps->hatX[idx-2].count; b++) {
                    if (areAdjacent(g, ps->M[idx-1].vertices[a], ps->hatX[idx-2].vertices[b]))
                        cnt++;
                }
                if (cnt != 1) ok[5] = 70;
            }
            for (int b = 0; b < ps->hatX[idx-2].count; b++) {
                int cnt = 0;
                for (int a = 0; a < ps->M[idx-1].count; a++) {
                    if (areAdjacent(g, ps->M[idx-1].vertices[a], ps->hatX[idx-2].vertices[b]))
                        cnt++;
                }
                if (cnt != 1) ok[5] = 0;
            }
        }
        if (i < n && ok[5]) {
            for (int a = 0; a < ps->M[idx+1].count; a++) {
                int cnt = 0;
                for (int b = 0; b < ps->hatX[idx+2].count; b++) {
                    if (areAdjacent(g, ps->M[idx+1].vertices[a], ps->hatX[idx+2].vertices[b]))
                        cnt++;
                }
                if (cnt != 1) ok[5] = 90;
            }
            for (int b = 0; b < ps->hatX[idx+2].count; b++) {
                int cnt = 0;
                for (int a = 0; a < ps->M[idx+1].count; a++) {
                    if (areAdjacent(g, ps->M[idx+1].vertices[a], ps->hatX[idx+2].vertices[b]))
                        cnt++;
                }
                if (cnt != 1) ok[5] = 100;
            }
        }
    }
    // Si aucun sommet avec |hatX|=1 n'existe, P6 est vraie par défaut
    // (ok[5] reste 1)

    // P7
    for (int i = 2; i < n && ok[6]; i++) {
        int idx = 2*i;
        if (ps->hatX[idx].count <= 1) continue;
        if (ps->R[idx-1].count != 0 || ps->L[idx+1].count != 0) { ok[6] = 0; break; }
        for (int a = 0; a < ps->X[idx-1].count && ok[6]; a++) {
            for (int b = 0; b < ps->X[idx+1].count && ok[6]; b++) {
                Vertex* u = ps->X[idx-1].vertices[a];
                Vertex* v = ps->X[idx+1].vertices[b];
                bool adj = areAdjacent(g, u, v);
                bool common = false;
                for (int k = 0; k < ps->hatX[idx].count; k++) {
                    Vertex* w = ps->hatX[idx].vertices[k];
                    if (areAdjacent(g, u, w) && areAdjacent(g, v, w)) {
                        common = true; break;
                    }
                }
                if (adj == common) { ok[6] = 0; break; }
            }
        }
    }
    // Si aucun i avec |hatX|>1, P7 est vraie par défaut

    return ok;
}

/* ---------- Fonction publique ---------- */
int* checkPathOfTriangleProperties(Graph* g) {
    PoTStructure ps;
    if (!buildPoTStructure(g, &ps,0)) {
        // La structure de base n'est pas valide -> toutes les propriétés fausses
        int* ok = malloc(7 * sizeof(int));
        if (ok) {
            for (int i = 0; i < 7; i++) ok[i] = 0;
        }
        return ok;
    }   
    int* result = checkProperties(g, &ps);
    //for (int i = 0; i < 7; i++) result[i] = 10;
    freePoTStructure(&ps);
    return result;
}