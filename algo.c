#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


/**
 * Sélectionne un sommet représentant pour chaque indice pair i du PathOfTriangle.
 * @param g    Graphe de type PathOfTriangle
 * @param path Nombre de sommets du chemin principal (path >= 1)
 * @return Tableau de path pointeurs de sommets (indice i -> représentant pour 2*(i+1))
 *         ou NULL si aucun sommet trouvé pour un indice. À libérer par l'appelant.
 */
Vertex** selectOneVertexPerIndex(Graph* g, int path) {
    Vertex** repr = malloc(path * sizeof(Vertex*));
    if (!repr) return NULL;

    // Initialiser à NULL
    for (int i = 0; i < path; i++) {
        repr[i] = NULL;
    }

    // Parcourir tous les sommets et retenir le premier trouvé pour chaque indice pair
    for (int v = 0; v < g->nbv; v++) {
        Vertex* vert = g->vertices[v];
        if (vert->name[0] != 's') continue;          // ne considérer que les "s..."
        int base = getBaseIndex(vert->name);         // récupère l'indice principal
        if (base % 2 != 0) continue;                 // ignorer les indices impairs
        if (base < 2 || base > 2 * path) continue;   // hors limites

        int idx = base / 2 - 1;                      // indice dans le tableau (0 pour s2, 1 pour s4, ...)
        if (repr[idx] == NULL) {
            repr[idx] = vert;                        // premier trouvé
        }
    }

    return repr;
}

#include <stdbool.h>

/* Structure pour stocker un triplet de sommets */
// Types pour l'algorithme
typedef struct _triplet{ Vertex* v[3]; struct _triplet* next;} Triplet;
typedef struct _pair{ Vertex* v[2]; struct _pair* next} Pair;
typedef struct _vertexCell {
    Vertex*             v;
    struct _vertexCell* next;
} VertexCell;


/**
 * Renvoie l’indice du sommet dans g->vertices, ou -1 s’il n’est pas trouvé.
 */
static int vertexIndex(Graph* g, Vertex* v) {
    for (int i = 0; i < g->nbv; i++) {
        if (g->vertices[i] == v) return i;
    }
    return -1;
}

/**
 * Vérifie si v est présent dans la liste chaînée list.
 */
static bool isVertexInList(Vertex* v, VertexCell* list) {
    while (list != NULL) {
        if (list->v == v) return true;
        list = list->next;
    }
    return false;
}

/**
 * Renvoie true si a et b sont reliés par une arête (voisins directs).
 * Utilise la liste d’adjacence de a (ou de b) pour un test en O(deg(a)).
 */
static bool areConnected(Graph* g, Vertex* a, Vertex* b) {
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

/**
 * Cherche deux voisins x, y de v tels que :
 *   - ni x ni y n’appartiennent à la liste interdite *forbidden*,
 *   - x et y sont reliés par une arête.
 *
 * @param g         Graphe (avec listes d’adjacence)
 * @param v         Sommet central
 * @param forbidden Liste chaînée des sommets interdits (par ex. les représentants xi)
 * @param x         (sortie) premier sommet trouvé
 * @param y         (sortie) second sommet trouvé
 * @return true si un couple (x,y) a été trouvé, false sinon.
 */
bool findTwoConnectedNeighbors(Graph* g, Vertex* v, VertexCell* forbidden,
                               Vertex** x, Vertex** y) {
    int v_idx = vertexIndex(g, v);
    if (v_idx == -1) return false;

    AdjList* adj = &g->adj[v_idx];

    // Tableau temporaire pour stocker les voisins éligibles
    Vertex** candidates = malloc(adj->size * sizeof(Vertex*));
    if (!candidates) return false;

    int nbCand = 0;
    for (int i = 0; i < adj->size; i++) {
        Edge* e = adj->edges[i];
        Vertex* neigh = (e->endpoints[0] == v) ? e->endpoints[1] : e->endpoints[0];
        if (!isVertexInList(neigh, forbidden)) {
            candidates[nbCand++] = neigh;
        }
    }

    // Chercher une paire connectée parmi ces voisins
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


/**
 * Cherche deux voisins de xi qui ne sont pas dans R (isR)
 * et qui sont connectés entre eux.
 */
static bool findTwoConnectedNeighbors(Graph* g, Vertex* xi, bool* isR,
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
        if (isR[nidx]) continue;   // ne pas prendre un autre représentant
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
typedef struct {
    Triplet* T;
    Pair* M;
    VertexCell* S
} Cover;


/**
 * Construit la liste des triplets T selon l'algorithme.
 * - Pour chaque xi non utilisé, on cherche deux voisins x,y non interdits (pas dans R)
 *   et connectés → triplet (xi, x, y).
 * - Si impossible, on prend xi et le représentant suivant xi+1, on cherche un voisin
 *   commun y (hors R) → triplet (xi, xi+1, y).
 *
 * @param g     Graphe
 * @param path  Nombre de sommets du chemin
 * @return      Liste chaînée de Triplet, NULL si aucun.
 */


Cover couveture(Graph* g, int path) {
    // 1. Représentants (un par indice pair)
    Vertex** R = selectOneVertexPerIndex(g, path);
    if (!R) return NULL;

    // 2. Tableau isR pour tester rapidement si un sommet est dans R
    int n = g->nbv;
    bool* isR = calloc(n, sizeof(bool));
    bool* used_xi = calloc(path, sizeof(bool));   // xi déjà placés dans T
    if (!isR || !used_xi) {
        free(R); free(isR); free(used_xi);
        return NULL;
    }
    for (int i = 0; i < path; i++) {
        if (R[i]) {
            int idx = vertexIndex(g, R[i]);
            if (idx != -1) isR[idx] = true;
        }
    }

    Triplet* T = NULL;

    // 3. Boucle principale sur les xi
    for (int i = 0; i < path; i++) {
        if (!R[i] || used_xi[i]) continue;   // xi inexistant ou déjà utilisé

        Vertex* xi = R[i];
        Vertex *x, *y;

        if (findTwoConnectedNeighbors(g, xi, isR, &x, &y)) {
            // Triplet (xi, x, y)
            Triplet* trip = malloc(sizeof(Triplet));
            if (!trip) break;
            trip->v[0] = xi;
            trip->v[1] = x;
            trip->v[2] = y;
            trip->next = T;
            T = trip;

            used_xi[i] = true;
        } else {
            // Essayer avec le xi suivant
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

    free(R);
    free(isR);
    free(used_xi);
    Pair M=NULL;
    VertexCell S=NUL;
    Cover sol;
    sol.T=T;
    sol.M=M;
    sol.S=S;
    return sol;
}




