#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "Graph.h"
#include "Algo.h"
#include "Matching.h"





/**
 * Compte le nombre d'éléments dans une liste chaînée de VertexCell.
 *
 * @param list  Tête de la liste (peut être NULL)
 * @return      Nombre de sommets dans la liste
 */
int countVertices(VertexCell* list) {
    int count = 0;
    while (list != NULL) {
        count++;
        list = list->next;
    }
    return count;
}

/**
 * Affiche les noms des sommets contenus dans un tableau de pointeurs Vertex*.
 * Les cases NULL sont affichées comme "(null)".
 *
 * @param array  Tableau de pointeurs de sommets
 * @param size   Nombre d'éléments du tableau
 */
void printVertexArray(Vertex** array, int size) {
    for (int i = 0; i < size; i++) {
        if (array[i] != NULL)
            printf("[%d] %s\n", i, array[i]->name);
        else
            printf("[%d] (null)\n", i);
    }
}

/**
 * Affiche les noms des sommets de chaque triplet de la liste T.
 * @param T  tête de la liste chaînée de Triplet (peut être NULL)
 */
void printTriplets(Triplet* T) {
    int idx = 1;
    for (Triplet* cur = T; cur != NULL; cur = cur->next, idx++) {
        printf("Triplet %d : %s, %s, %s\n", idx,
               cur->v[0]->name, cur->v[1]->name, cur->v[2]->name);
    }
}


/* ---------------- selectOneVertexPerIndex ---------------- */
Vertex** selectOneVertexPerIndex(Graph* g, int path) {
    Vertex** repr = malloc(path * sizeof(Vertex*));
    if (!repr) return NULL;

    for (int i = 0; i < path; i++) {
        repr[i] = NULL;
    }

    for (int v = 0; v < g->nbv; v++) {
        Vertex* vert = g->vertices[v];
        if (vert->name[0] != 's') continue;
        int base = getBaseIndex(vert->name);
        if (base % 2 != 0) continue;
        if (base < 2 || base > 2 * path) continue;

        int idx = base / 2 - 1;
        if (repr[idx] == NULL) {
            repr[idx] = vert;
        }
    }

    return repr;
}



bool isVertexInList(Vertex* v, VertexCell* list) {
    while (list != NULL) {
        if (list->v == v) return true;
        list = list->next;
    }
    return false;
}

 bool areConnected(Graph* g, Vertex* a, Vertex* b) {
    int a_idx = vertexIndex(g, a);
    if (a_idx == -1) return false;

    AdjList* adj = &g->adj[a_idx];
    for (int i = 0; i < adj->size; i++) {
        Edge* e = adj->edges[i];
        Vertex* other = (e->endpoints[0] == a) ? e->endpoints[1] : e->endpoints[0];
        if (other == b) return true;
    }
    return false;
}



 bool findTwoConnectedNeighbors(Graph* g, Vertex* xi, bool* isR,
                                      Vertex** x, Vertex** y) {
    int xi_idx = vertexIndex(g, xi);
    if (xi_idx == -1) return false;

    AdjList* adj = &g->adj[xi_idx];
    Vertex** candidates = malloc(adj->size * sizeof(Vertex*));
    if (!candidates) return false;

    int nbCand = 0;
    for (int i = 0; i < adj->size; i++) {
        Edge* e = adj->edges[i];
        Vertex* neigh = (e->endpoints[0] == xi) ? e->endpoints[1] : e->endpoints[0];
        int nidx = vertexIndex(g, neigh);
        if (nidx == -1) continue;
        if (isR[nidx]) continue;
        candidates[nbCand++] = neigh;
    }

    bool found = false;
    for (int i = 0; i < nbCand && !found; i++) {
        for (int j = i + 1; j < nbCand && !found; j++) {
            if (areConnected(g, candidates[i], candidates[j])) {
                *x = candidates[i];
                *y = candidates[j];
                found = true;
            }
        }
    }

    free(candidates);
    return found;
}

/* ---------------- removeVerticesFromTriplets ---------------- */
Graph* removeVerticesFromTriplets(Graph* g, Triplet* T) {
    if (T == NULL) {
        return g;
    }
    deleteVertex(g, *(T->v[0]));
    deleteVertex(g, *(T->v[1]));
    deleteVertex(g, *(T->v[2]));
    return removeVerticesFromTriplets(g, T->next);
}



Vertex* findCommonNeighbor(Graph* g, Vertex* a, Vertex* b) {
    int idx_a = vertexIndex(g, a);
    int idx_b = vertexIndex(g, b);
    if (idx_a == -1 || idx_b == -1) return NULL;

    int n = g->nbv;
    bool* marked = calloc(n, sizeof(bool));
    if (!marked) return NULL;

    // Marquer les voisins de a
    AdjList* adj_a = &g->adj[idx_a];
    for (int i = 0; i < adj_a->size; i++) {
        Edge* e = adj_a->edges[i];
        Vertex* neigh = (e->endpoints[0] == a) ? e->endpoints[1] : e->endpoints[0];
        int idx_n = vertexIndex(g, neigh);
        if (idx_n != -1) marked[idx_n] = true;
    }

    // Chercher un voisin de b marqué
    Vertex* common = NULL;
    AdjList* adj_b = &g->adj[idx_b];
    for (int i = 0; i < adj_b->size; i++) {
        Edge* e = adj_b->edges[i];
        Vertex* neigh = (e->endpoints[0] == b) ? e->endpoints[1] : e->endpoints[0];
        int idx_n = vertexIndex(g, neigh);
        if (idx_n != -1 && marked[idx_n]) {
            common = neigh;
            break;
        }
    }

    free(marked);
    return common;
}








/**
 * Construit les listes M (paires couplées) et S (sommets non couplés)
 * à partir du graphe gprime en utilisant l'algorithme d'Edmonds.
 *
 * @param gprime  Graphe d'entrée (déjà élagué)
 * @param M       (sortie) pointeur vers la tête de la liste chaînée de Pair
 * @param S       (sortie) pointeur vers la tête de la liste chaînée de VertexCell
 */
void buildMatchingLists(Graph* gprime, Pair** M, VertexCell** S) {
    int n = gprime->nbv;
    int* match = malloc(n * sizeof(int));
    if (!match) return;

    int size = maximumMatching(gprime, match);
    printf("Couplage maximum trouvé : %d arêtes\n", size);

    *M = NULL;
    *S = NULL;

    // Marqueur pour éviter de dupliquer les paires
    bool* used = calloc(n, sizeof(bool));
    if (!used) {
        free(match);
        return;
    }

    // 1. Parcourir le tableau match et créer une Pair pour chaque arête du couplage
    for (int i = 0; i < n; i++) {
        if (used[i]) continue;
        int j = match[i];
        if (j != -1 && !used[j]) {
            Pair* p = malloc(sizeof(Pair));
            if (!p) { free(used); free(match); return; }
            p->v[0] = gprime->vertices[i];
            p->v[1] = gprime->vertices[j];
            p->next = *M;
            *M = p;

            used[i] = true;
            used[j] = true;
        }
    }

    // 2. Les sommets non couplés vont dans S
    for (int i = 0; i < n; i++) {
        if (match[i] == -1) {
            VertexCell* cell = malloc(sizeof(VertexCell));
            if (!cell) { /* gestion erreur */ continue; }
            cell->v = gprime->vertices[i];
            cell->next = *S;
            *S = cell;
        }
    }

    free(used);
    free(match);
}

/* ---------------- couveture ---------------- */
Cover couveture(Graph* g, int path) {
    Cover sol = {NULL, NULL, NULL};

    Vertex** R = selectOneVertexPerIndex(g, path);
    if (!R) return sol;

    int n = g->nbv;
    bool* isR = calloc(n, sizeof(bool));
    bool* used_xi = calloc(path, sizeof(bool));
    if (!isR || !used_xi) {
        free(R); free(isR); free(used_xi);
        return sol;
    }
    for (int i = 0; i < path; i++) {
        if (R[i]) {
            int idx = vertexIndex(g, R[i]);
            if (idx != -1) isR[idx] = true;
        }
    }

    Triplet* T = NULL;

    for (int i = 0; i < path; i++) {
        if (!R[i] || used_xi[i]) continue;

        Vertex* xi = R[i];
        Vertex *x, *y;

        if (findTwoConnectedNeighbors(g, xi, isR, &x, &y)) {
            Triplet* trip = malloc(sizeof(Triplet));
            if (!trip) break;
            trip->v[0] = xi;
            trip->v[1] = x;
            trip->v[2] = y;
            trip->next = T;
            T = trip;

            used_xi[i] = true;
        } else {
            if (i + 1 < path && R[i+1] && !used_xi[i+1]) {
                Vertex* xi_next = R[i+1];
                Vertex* y = findCommonNeighbor(g, xi, xi_next);
                if (y != NULL) {
                    Triplet* trip = malloc(sizeof(Triplet));
                    if (!trip) break;
                    trip->v[0] = xi;
                    trip->v[1] = xi_next;
                    trip->v[2] = y;
                    trip->next = T;
                    T = trip;

                    used_xi[i] = true;
                    used_xi[i+1] = true;
                }
            }
        }
    }
    
    Graph* gprime = cloneGraph(g);

    gprime = removeVerticesFromTriplets(gprime, T);  
    generateDotFile(gprime, "graphes/test.dot");
    printf("NOMBRE DE TRIANGLES APRES RETIRER LES TRIANGLES: %d\n", countTriangle(gprime));
    

    Pair* M = NULL;
    VertexCell* S = NULL;

    buildMatchingLists(gprime, &M, &S);

    

    
       
    
    sol.T = T;
    sol.M = M;
    sol.S = S;
   
    freeGraph(gprime); 
    free(R);
    free(isR);
    free(used_xi);
    
    return sol;
}