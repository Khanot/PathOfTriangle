    #ifndef ALGO_H
    #define ALGO_H

    #include <stdbool.h>
    #include "Graph.h"





    /**
     * Liste de paires candidates pour un xi.
     * Contient jusqu'à maxCandidates paires (x,y), et un booléen indiquant si le secours (xi+1 + voisin commun) est possible.
     */
    typedef struct {
        Vertex* xi;               // sommet central
        Vertex* xi_next;          // pour l'option de secours, sinon NULL
        Vertex** x;               // tableau de x
        Vertex** y;               // tableau de y
        int nbPairs;              // nombre de paires (x,y)
        bool secoursPossible;     // true si le secours est disponible
        Vertex* secoursY;         // voisin commun pour le secours (si possible)
    } CandidateList;

    typedef struct _coverListNode {
        Cover cover;
        struct _coverListNode* next;
    } CoverListNode;






    /* ---------- Prototypes des fonctions publiques ---------- */

    int countTriplets(Triplet* T);
    int countPairs(Pair* M);
    void printCoverListSizes(CoverListNode* list);
    void printTriplets(Triplet* T);
    void printVertexArray(Vertex** array, int size);
    int countVertices(VertexCell* list);
    int countIsolatedVertices(Cover* c);

    // Sélectionne un sommet représentant par indice pair (chemin de triangles)
    Vertex** selectOneVertexPerIndex(Graph* g, int path);



    // Teste si un sommet est présent dans une liste chaînée
    bool isVertexInList(Vertex* v, VertexCell* list);

    // Teste si deux sommets sont reliés par une arête (utilise les listes d’adjacence)
    bool areConnected(Graph* g, Vertex* a, Vertex* b);



    // Cherche deux voisins de xi hors des représentants (tableau isR), connectés entre eux
    bool findTwoConnectedNeighbors(Graph* g, Vertex* xi, bool* isR,
                                Vertex** x, Vertex** y);

    bool findTwoConnectedNeighborsM(Graph* g, Vertex* xi, bool* isR,
                                    Vertex** x, Vertex** y);

    bool findTwoConnectedNeighborsLR(Graph* g, Vertex* xi, bool* isR,
                                    Vertex** x, Vertex** y);

    // Supprime récursivement les sommets des triplets (utilise deleteVertex)
    Graph* removeVerticesFromTriplets(Graph* g, Triplet* T);


    Vertex* findCommonNeighbor(Graph* g, Vertex* a, Vertex* b);







    void buildMatchingLists(Graph* gprime, Pair** M, VertexCell** S);




    // Fonction principale de couverture : calcule T (M et S non implémentés)
    Cover couveture(Graph* g, int path);
    Cover couvetureLR(Graph* g, int path);
    Cover couvetureM(Graph* g, int path);



    Cover couvertureExhaustive(Graph* g, int path);















VertexCell* cloneVertexCellList(VertexCell* S);
Pair* clonePairList(Pair* M) ;
Triplet* cloneTripletList(Triplet* T);

void backtrackAll(Graph* g, Vertex** R, int path, bool* isR,
                         int idx, Triplet* T, bool* used_xi, bool* used_vertices,
                         CoverListNode** list);
void addCoverToList(CoverListNode** list, Graph* g, Triplet* T);


CoverListNode* couvertureAll(Graph* g, int path);

void freeTripletList(Triplet* T);
void freePairList(Pair* M);
void freeVertexCellList(VertexCell* S);
void freeCover(Cover* c);

void freeCoverList(CoverListNode* list);




Cover* getNthCover(CoverListNode* list, int n);

void printIsolatedVertices(Cover* cover);

Cover couvetureManuelle(Graph* g, int path);
void buildMatchingListsManuelle(Graph* gprime, Pair** M, VertexCell** S, int path);

#endif /* ALGO_H */