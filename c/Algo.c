#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "Graph.h"
#include "Algo.h"
#include "Matching.h"
#include "Utils.h"




/**
 * Compte le nombre de triplets dans une liste chaînée.
 */
int countTriplets(Triplet* T) {
    int cnt = 0;
    while (T) { cnt++; T = T->next; }
    return cnt;
}

/**
 * Compte le nombre de paires dans une liste chaînée.
 */
int countPairs(Pair* M) {
    int cnt = 0;
    while (M) { cnt++; M = M->next; }
    return cnt;
}
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
 * Affiche les tailles [T,P,V] de chaque couverture contenue dans la liste.
 * T = nombre de triplets, P = nombre de paires du couplage, V = nombre d'isolés.
 */
void printCoverListSizes(CoverListNode* list) {
    int idx = 1;
    for (CoverListNode* cur = list; cur != NULL; cur = cur->next, idx++) {
        int t = countTriplets(cur->cover.T);
        int p = countPairs(cur->cover.M);
        int v = countVertices(cur->cover.S);
        printf("Couverture %d : [%d, %d, %d -> %d]\n", idx, t, p, v,t+p+v);
    }
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
        if (strchr(vert->name, '_') != NULL) continue;
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
    //generateDotFile(gprime, "graphes/test.dot");
    //printf("NOMBRE DE TRIANGLES APRES RETIRER LES TRIANGLES: %d\n", countTriangle(gprime));
    

    Pair* M = NULL;
    VertexCell* S = NULL;

    buildMatchingLists(gprime, &M, &S);

    relinkPairs(M, g);
    relinkVertexCells(S, g);
    sol.T = T;
    sol.M = M;
    sol.S = S;
   
    freeGraph(gprime); 
    free(R);
    free(isR);
    free(used_xi);
    
    return sol;
}

/**
 * Cherche deux voisins de xi hors des représentants (isR), connectés entre eux.
 * Priorité aux sommets dont le nom commence par 'l' ou 'r'.
 * Si aucune paire avec 'l'/'r' n'existe, retourne n'importe quelle paire valide.
 *
 * @return true si une paire (x,y) est trouvée, false sinon.
 */
bool findTwoConnectedNeighborsLR(Graph* g, Vertex* xi, bool* isR,
                                 Vertex** x, Vertex** y) {
    int xi_idx = xi->id;
    AdjList* adj = &g->adj[xi_idx];
    int nbCand = 0;
    Vertex** candidates = malloc(adj->size * sizeof(Vertex*));
    if (!candidates) return false;

    // 1. Collecter tous les voisins éligibles (hors R)
    for (int i = 0; i < adj->size; i++) {
        Edge* e = adj->edges[i];
        Vertex* neigh = (e->endpoints[0] == xi) ? e->endpoints[1] : e->endpoints[0];
        if (!isR[neigh->id]) {
            candidates[nbCand++] = neigh;
        }
    }

    // 2. Chercher d'abord une paire connectée contenant au moins un 'l' ou 'r'
    for (int i = 0; i < nbCand; i++) {
        for (int j = i + 1; j < nbCand; j++) {
            Vertex* a = candidates[i];
            Vertex* b = candidates[j];
            if (areConnected(g, a, b)) {
                if (a->name[0] == 'l' || a->name[0] == 'r' ||
                    b->name[0] == 'l' || b->name[0] == 'r') {
                    *x = a;
                    *y = b;
                    free(candidates);
                    return true;
                }
            }
        }
    }

    // 3. Sinon, chercher n'importe quelle paire connectée
    for (int i = 0; i < nbCand; i++) {
        for (int j = i + 1; j < nbCand; j++) {
            if (areConnected(g, candidates[i], candidates[j])) {
                *x = candidates[i];
                *y = candidates[j];
                free(candidates);
                return true;
            }
        }
    }

    free(candidates);
    return false;
}
Cover couvetureLR(Graph* g, int path) {
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

        if (findTwoConnectedNeighborsLR(g, xi, isR, &x, &y)) {
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
    //generateDotFile(gprime, "graphes/test.dot");
    //printf("NOMBRE DE TRIANGLES APRES RETIRER LES TRIANGLES: %d\n", countTriangle(gprime));
    

    Pair* M = NULL;
    VertexCell* S = NULL;

    buildMatchingLists(gprime, &M, &S);
    relinkPairs(M, g);
    relinkVertexCells(S, g);
    
    sol.T = T;
    sol.M = M;
    sol.S = S;
   
    freeGraph(gprime); 
    free(R);
    free(isR);
    free(used_xi);
    
    return sol;
}
/**
 * Cherche deux voisins de xi hors des représentants (isR), connectés entre eux.
 * Priorité : un sommet dont le nom commence par 'm'.
 * Sinon, n'importe quelle paire valide.
 *
 * @return true si une paire (x,y) est trouvée, false sinon.
 */
bool findTwoConnectedNeighborsM(Graph* g, Vertex* xi, bool* isR,
                                Vertex** x, Vertex** y) {
    int xi_idx = xi->id;
    AdjList* adj = &g->adj[xi_idx];
    Vertex** candidates = malloc(adj->size * sizeof(Vertex*));
    if (!candidates) return false;

    int nbCand = 0;
    // Collecter les voisins hors R
    for (int i = 0; i < adj->size; i++) {
        Edge* e = adj->edges[i];
        Vertex* neigh = (e->endpoints[0] == xi) ? e->endpoints[1] : e->endpoints[0];
        if (!isR[neigh->id]) {
            candidates[nbCand++] = neigh;
        }
    }

    // 1. Chercher une paire connectée contenant au moins un 'm'
    for (int i = 0; i < nbCand; i++) {
        for (int j = i + 1; j < nbCand; j++) {
            if (areConnected(g, candidates[i], candidates[j])) {
                if (candidates[i]->name[0] == 'm' || candidates[j]->name[0] == 'm') {
                    *x = candidates[i];
                    *y = candidates[j];
                    free(candidates);
                    return true;
                }
            }
        }
    }

    // 2. Sinon, n'importe quelle paire connectée
    for (int i = 0; i < nbCand; i++) {
        for (int j = i + 1; j < nbCand; j++) {
            if (areConnected(g, candidates[i], candidates[j])) {
                *x = candidates[i];
                *y = candidates[j];
                free(candidates);
                return true;
            }
        }
    }

    free(candidates);
    return false;
}
Cover couvetureM(Graph* g, int path) {
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

        if (findTwoConnectedNeighborsM(g, xi, isR, &x, &y)) {
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
    //generateDotFile(gprime, "graphes/test.dot");
    //printf("NOMBRE DE TRIANGLES APRES RETIRER LES TRIANGLES: %d\n", countTriangle(gprime));
    

    Pair* M = NULL;
    VertexCell* S = NULL;

    buildMatchingLists(gprime, &M, &S);
    relinkPairs(M, g);
    relinkVertexCells(S, g);
    
    sol.T = T;
    sol.M = M;
    sol.S = S;
   
    freeGraph(gprime); 
    free(R);
    free(isR);
    free(used_xi);
    
    return sol;
}



/**
 * Remplit la CandidateList pour un xi donné.
 * - Toutes les paires (x,y) de voisins (hors R, non déjà utilisés) connectés entre eux.
 * - Option de secours : xi+1 + findCommonNeighbor(xi, xi_next) si xi_next non utilisé et y non utilisé.
 *
 * @param used_vertices : tableau de booléens de taille g->nbv indiquant les sommets déjà utilisés dans un triangle.
 */
CandidateList listCandidates(Graph* g, Vertex* xi, bool* isR, bool* used_vertices,
                             Vertex** R, int path, int i) {
    CandidateList cl;
    cl.xi = xi;
    cl.xi_next = NULL;
    cl.x = NULL; cl.y = NULL;
    cl.nbPairs = 0;
    cl.secoursPossible = false;
    cl.secoursY = NULL;

    int xi_idx = xi->id;
    AdjList* adj = &g->adj[xi_idx];
    
    // Collecter les voisins éligibles (hors R, hors déjà utilisés)
    int nbCand = 0;
    Vertex** candidates = malloc(adj->size * sizeof(Vertex*));
    for (int k = 0; k < adj->size; k++) {
        Edge* e = adj->edges[k];
        Vertex* neigh = (e->endpoints[0] == xi) ? e->endpoints[1] : e->endpoints[0];
        if (!isR[neigh->id] && !used_vertices[neigh->id]) {
            candidates[nbCand++] = neigh;
        }
    }

    // Allouer les tableaux x,y à la taille max possible (nbCand*(nbCand-1)/2)
    int maxPairs = nbCand * (nbCand - 1) / 2;
    if (maxPairs > 0) {
        cl.x = malloc(maxPairs * sizeof(Vertex*));
        cl.y = malloc(maxPairs * sizeof(Vertex*));
        // Remplir toutes les paires connectées
        for (int a = 0; a < nbCand; a++) {
            for (int b = a + 1; b < nbCand; b++) {
                if (areConnected(g, candidates[a], candidates[b])) {
                    cl.x[cl.nbPairs] = candidates[a];
                    cl.y[cl.nbPairs] = candidates[b];
                    cl.nbPairs++;
                }
            }
        }
    }

    free(candidates);

    // Option de secours : xi_next = R[i+1] si disponible
    if (i + 1 < path && R[i+1] != NULL && !used_vertices[R[i+1]->id]) {
        Vertex* xi_next = R[i+1];
        Vertex* y = findCommonNeighbor(g, xi, xi_next);
        if (y != NULL && !isR[y->id] && !used_vertices[y->id]) {
            cl.xi_next = xi_next;
            cl.secoursPossible = true;
            cl.secoursY = y;
        }
    }

    return cl;
}


/**
 * Backtracking récursif pour explorer toutes les couvertures.
 *
 * @param idx        : premier indice dans R à partir duquel chercher un xi non utilisé.
 * @param T          : liste chaînée des triplets en cours.
 * @param used_xi    : tableau booléen des xi déjà traités.
 * @param used_vertices : tableau booléen des sommets déjà utilisés dans un triangle.
 * @param bestCover  : meilleure couverture trouvée jusqu'ici (score max de S).
 * @param bestScore  : score de la meilleure couverture (nombre de sommets dans S).
 */
void backtrack(Graph* g, Vertex** R, int path, bool* isR,
                      int idx, Triplet* T, bool* used_xi, bool* used_vertices,
                      Cover* bestCover, int* bestScore) {
    // Chercher le prochain xi non utilisé
    while (idx < path && (R[idx] == NULL || used_xi[idx]))
        idx++;
    if (idx >= path) {
        // Tous les xi sont couverts : évaluer la solution
        Graph* gcopy = cloneGraph(g);
        gcopy = removeVerticesFromTriplets(gcopy, T);
        Pair* M = NULL;
        VertexCell* S = NULL;
        buildMatchingLists(gcopy, &M, &S);
        relinkPairs(M, g);
        relinkVertexCells(S, g);
        int score = countVertices(S);
        if (score > *bestScore) {
            // Libérer l'ancienne meilleure couverture
            freeCover(bestCover);
            // Copier la nouvelle
            bestCover->T = cloneTripletList(T);   // copie profonde
            bestCover->M = clonePairList(M);
            bestCover->S = cloneVertexCellList(S);
            *bestScore = score;
        }
        // Libérer ce qu'on vient de créer
        freePairList(M);
        freeVertexCellList(S);
        freeGraph(gcopy);
        return;
    }

    Vertex* xi = R[idx];

    // 1. Obtenir toutes les options pour ce xi
    CandidateList cand = listCandidates(g, xi, isR, used_vertices, R, path, idx);

    // 2. Essayer chaque paire (x,y)
    for (int p = 0; p < cand.nbPairs; p++) {
        Vertex* x = cand.x[p];
        Vertex* y = cand.y[p];
        // Marquer xi, x, y comme utilisés
        used_xi[idx] = true;
        used_vertices[xi->id] = true;
        used_vertices[x->id] = true;
        used_vertices[y->id] = true;

        Triplet* trip = malloc(sizeof(Triplet));
        trip->v[0] = xi;
        trip->v[1] = x;
        trip->v[2] = y;
        trip->next = T;

        backtrack(g, R, path, isR, idx + 1, trip, used_xi, used_vertices,
                  bestCover, bestScore);

        // Désallouer le triplet courant (pas son contenu, les sommets ne sont pas libérés)
        free(trip);
        // Démasquer
        used_xi[idx] = false;
        used_vertices[xi->id] = false;
        used_vertices[x->id] = false;
        used_vertices[y->id] = false;
    }

    // 3. Option de secours (xi + xi_next + y)
    if (cand.secoursPossible) {
        Vertex* xi_next = cand.xi_next;
        Vertex* y = cand.secoursY;
        int idx_next = idx + 1;   // xi_next est à l'indice idx+1

        used_xi[idx] = true;
        used_xi[idx_next] = true;
        used_vertices[xi->id] = true;
        used_vertices[xi_next->id] = true;
        used_vertices[y->id] = true;

        Triplet* trip = malloc(sizeof(Triplet));
        trip->v[0] = xi;
        trip->v[1] = xi_next;
        trip->v[2] = y;
        trip->next = T;

        backtrack(g, R, path, isR, idx + 1, trip, used_xi, used_vertices,
                  bestCover, bestScore);

        free(trip);
        used_xi[idx] = false;
        used_xi[idx_next] = false;
        used_vertices[xi->id] = false;
        used_vertices[xi_next->id] = false;
        used_vertices[y->id] = false;
    }

    // Nettoyage des listes de candidats
    free(cand.x);
    free(cand.y);
}

/**
 * Couverture exhaustive qui explore toutes les combinaisons de triangles.
 * Retourne la couverture qui maximise le nombre de sommets isolés dans S.
 */
Cover couvertureExhaustive(Graph* g, int path) {
    Cover best = {NULL, NULL, NULL};
    int bestScore = -1;

    Vertex** R = selectOneVertexPerIndex(g, path);
    if (!R) return best;

    int n = g->nbv;
    bool* isR = calloc(n, sizeof(bool));
    bool* used_xi = calloc(path, sizeof(bool));
    bool* used_vertices = calloc(n, sizeof(bool));
    if (!isR || !used_xi || !used_vertices) {
        free(R); free(isR); free(used_xi); free(used_vertices);
        return best;
    }
    for (int i = 0; i < path; i++) {
        if (R[i]) {
            int idx = vertexIndex(g, R[i]);
            if (idx != -1) isR[idx] = true;
        }
    }

    backtrack(g, R, path, isR, 0, NULL, used_xi, used_vertices,
              &best, &bestScore);

    free(R); free(isR); free(used_xi); free(used_vertices);
    return best;
}

Triplet* cloneTripletList(Triplet* T) {
    if (!T) return NULL;
    Triplet* head = NULL, *tail = NULL;
    while (T) {
        Triplet* n = malloc(sizeof(Triplet));
        memcpy(n->v, T->v, 3 * sizeof(Vertex*));
        n->next = NULL;
        if (!head) head = tail = n;
        else { tail->next = n; tail = n; }
        T = T->next;
    }
    return head;
}

Pair* clonePairList(Pair* M) {
    if (!M) return NULL;
    Pair* head = NULL, *tail = NULL;
    while (M) {
        Pair* n = malloc(sizeof(Pair));
        memcpy(n->v, M->v, 2 * sizeof(Vertex*));
        n->next = NULL;
        if (!head) head = tail = n;
        else { tail->next = n; tail = n; }
        M = M->next;
    }
    return head;
}

VertexCell* cloneVertexCellList(VertexCell* S) {
    if (!S) return NULL;
    VertexCell* head = NULL, *tail = NULL;
    while (S) {
        VertexCell* n = malloc(sizeof(VertexCell));
        n->v = S->v;
        n->next = NULL;
        if (!head) head = tail = n;
        else { tail->next = n; tail = n; }
        S = S->next;
    }
    return head;
}

/**
 * Ajoute une couverture complète dans la liste chaînée.
 * Construit le graphe résiduel, calcule M et S via buildMatchingLists,
 * puis insère en tête de liste.
 */
void addCoverToList(CoverListNode** list, Graph* g, Triplet* T) {
    Graph* gcopy = cloneGraph(g);
    gcopy = removeVerticesFromTriplets(gcopy, T);

    Pair* M = NULL;
    VertexCell* S = NULL;
    buildMatchingLists(gcopy, &M, &S);
    relinkPairs(M, g);
    relinkVertexCells(S, g);
    Cover cover;
    cover.T = cloneTripletList(T);
    cover.M = M;
    cover.S = S;


    CoverListNode* node = malloc(sizeof(CoverListNode));
    node->cover = cover;
    node->next = *list;
    *list = node;

    freeGraph(gcopy);
}

void backtrackAll(Graph* g, Vertex** R, int path, bool* isR,
                         int idx, Triplet* T, bool* used_xi, bool* used_vertices,
                         CoverListNode** list) {
    // Chercher le prochain xi non utilisé
    while (idx < path && (R[idx] == NULL || used_xi[idx]))
        idx++;
    if (idx >= path) {
        // Tous les xi sont couverts : ajouter cette solution
        addCoverToList(list, g, T);
        return;
    }

    Vertex* xi = R[idx];
    CandidateList cand = listCandidates(g, xi, isR, used_vertices, R, path, idx);

    // Essayer chaque paire (x,y)
    for (int p = 0; p < cand.nbPairs; p++) {
        Vertex* x = cand.x[p];
        Vertex* y = cand.y[p];
        // Marquer xi, x, y comme utilisés
        used_xi[idx] = true;
        used_vertices[xi->id] = true;
        used_vertices[x->id] = true;
        used_vertices[y->id] = true;

        Triplet* trip = malloc(sizeof(Triplet));
        trip->v[0] = xi;
        trip->v[1] = x;
        trip->v[2] = y;
        trip->next = T;

        backtrackAll(g, R, path, isR, idx + 1, trip, used_xi, used_vertices, list);

        free(trip);
        used_xi[idx] = false;
        used_vertices[xi->id] = false;
        used_vertices[x->id] = false;
        used_vertices[y->id] = false;
    }

    // Option de secours (xi + xi_next + y)
    if (cand.secoursPossible) {
        Vertex* xi_next = cand.xi_next;
        Vertex* y = cand.secoursY;
        int idx_next = idx + 1;

        used_xi[idx] = true;
        used_xi[idx_next] = true;
        used_vertices[xi->id] = true;
        used_vertices[xi_next->id] = true;
        used_vertices[y->id] = true;

        Triplet* trip = malloc(sizeof(Triplet));
        trip->v[0] = xi;
        trip->v[1] = xi_next;
        trip->v[2] = y;
        trip->next = T;

        backtrackAll(g, R, path, isR, idx + 1, trip, used_xi, used_vertices, list);

        free(trip);
        used_xi[idx] = false;
        used_xi[idx_next] = false;
        used_vertices[xi->id] = false;
        used_vertices[xi_next->id] = false;
        used_vertices[y->id] = false;
    }

    free(cand.x);
    free(cand.y);
}

/**
 * Retourne une liste chaînée de toutes les couvertures possibles.
 * Chaque couverture contient les triplets T, le couplage M et les isolés S
 * du graphe après suppression des triplets.
 */
CoverListNode* couvertureAll(Graph* g, int path) {
    CoverListNode* list = NULL;

    Vertex** R = selectOneVertexPerIndex(g, path);
    if (!R) return NULL;

    int n = g->nbv;
    bool* isR = calloc(n, sizeof(bool));
    bool* used_xi = calloc(path, sizeof(bool));
    bool* used_vertices = calloc(n, sizeof(bool));
    if (!isR || !used_xi || !used_vertices) {
        free(R); free(isR); free(used_xi); free(used_vertices);
        return NULL;
    }
    for (int i = 0; i < path; i++) {
        if (R[i]) {
            int idx = vertexIndex(g, R[i]);
            if (idx != -1) isR[idx] = true;
        }
    }

    backtrackAll(g, R, path, isR, 0, NULL, used_xi, used_vertices, &list);

    free(R); free(isR); free(used_xi); free(used_vertices);
    return list;
}

// Libération d'une liste de CoverListNode

void freeTripletList(Triplet* T) {
    while (T) { Triplet* n = T->next; free(T); T = n; }
}
void freePairList(Pair* M) {
    while (M) { Pair* n = M->next; free(M); M = n; }
}
void freeVertexCellList(VertexCell* S) {
    while (S) { VertexCell* n = S->next; free(S); S = n; }
}
void freeCover(Cover* c) {
    freeTripletList(c->T);
    freePairList(c->M);
    freeVertexCellList(c->S);
}
void freeCoverList(CoverListNode* list) {
    while (list) {
        CoverListNode* next = list->next;
        freeCover(&list->cover);   // libère T, M, S
        free(list);
        list = next;
    }
}

/**
 * Retourne un pointeur vers la n-ième couverture de la liste (n commence à 1).
 * Renvoie NULL si n est inférieur à 1 ou si la liste contient moins de n éléments.
 * La mémoire de la couverture reste gérée par la liste, ne pas libérer le pointeur retourné.
 */
Cover* getNthCover(CoverListNode* list, int n) {
    int i = 1;
    while (list != NULL) {
        if (i == n) return &(list->cover);
        list = list->next;
        i++;
    }
    return NULL;
}