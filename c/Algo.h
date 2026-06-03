#ifndef ALGO_H
#define ALGO_H

#include <stdbool.h>
#include "Graph.h"

/* ---------- Types pour l'algorithme ---------- */
typedef struct _triplet {
    Vertex* v[3];
    struct _triplet* next;
} Triplet;

typedef struct _pair {
    Vertex* v[2];
    struct _pair* next;
} Pair;

typedef struct _vertexCell {
    Vertex*             v;
    struct _vertexCell* next;
} VertexCell;

typedef struct {
    Triplet*   T;
    Pair*      M;
    VertexCell* S;
} Cover;

/* ---------- Prototypes des fonctions publiques ---------- */


void printTriplets(Triplet* T);
void printVertexArray(Vertex** array, int size);
int countVertices(VertexCell* list);


// Sélectionne un sommet représentant par indice pair (chemin de triangles)
Vertex** selectOneVertexPerIndex(Graph* g, int path);



// Teste si un sommet est présent dans une liste chaînée
bool isVertexInList(Vertex* v, VertexCell* list);

// Teste si deux sommets sont reliés par une arête (utilise les listes d’adjacence)
bool areConnected(Graph* g, Vertex* a, Vertex* b);



// Cherche deux voisins de xi hors des représentants (tableau isR), connectés entre eux
bool findTwoConnectedNeighbors(Graph* g, Vertex* xi, bool* isR,
                               Vertex** x, Vertex** y);

// Supprime récursivement les sommets des triplets (utilise deleteVertex)
Graph* removeVerticesFromTriplets(Graph* g, Triplet* T);


Vertex* findCommonNeighbor(Graph* g, Vertex* a, Vertex* b);







void buildMatchingLists(Graph* gprime, Pair** M, VertexCell** S);




// Fonction principale de couverture : calcule T (M et S non implémentés)
Cover couveture(Graph* g, int path);

#endif /* ALGO_H */