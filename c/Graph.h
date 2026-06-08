#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define BUFFER_SIZE 256

/* ---------- Structures de données ---------- */

// Dans Graph.h, modifier la structure Vertex
typedef struct _vertex {
    char* name;
    int id;      // indice dans le tableau g->vertices
} Vertex;

typedef struct _edge {
    char* name;
    Vertex* endpoints[2];   // les deux sommets incidents
} Edge;

/* Liste d'adjacence dynamique pour un sommet */
typedef struct {
    Edge** edges;           // tableau de pointeurs d'arêtes
    int size;               // nombre actuel d'éléments
    int capacity;           // capacité allouée
} AdjList;

typedef struct _graph {
    char* name;
    int nbvMAX;             // capacité maximale en sommets
    int nbv;                // nombre actuel de sommets
    Vertex** vertices;      // tableau de pointeurs de sommets
    int nbeMAX;             // capacité maximale en arêtes
    int nbe;                // nombre actuel d'arêtes
    Edge** edges;           // tableau de pointeurs d'arêtes
    AdjList* adj;           // listes d'adjacence (une par sommet)
} Graph;

/* Résultat d'une requête de voisinage */
typedef struct {
    Vertex* v;              // le sommet central
    Edge** issuedEdges;     // ses arêtes incidentes
    int size;               // nombre d'arêtes
} IssuedEdges;


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



/* ---------- Création des éléments ---------- */

Vertex* createVertex(char* name);
Edge* createEdge(char* name, Vertex* v1, Vertex* v2);
Graph* createGraph(char* name, int nbvMAX);

/* ---------- Recherche ---------- */

Vertex* getVertex(Graph* g, char* name);
Edge* getEdge(Graph* g, char* name);

/* ---------- Modification du graphe ---------- */

int addVertex(Graph* g, Vertex* v);
int addEdge(Graph* g, Edge* e);
void deleteEdge(Graph* g, Edge e);
void deleteVertex(Graph* g, Vertex v);

/* ---------- Voisinage ---------- */

IssuedEdges* getIssuedEdges(Graph* g, Vertex* v);
void freeIssuedEdges(IssuedEdges* n);

/* ---------- Utilitaires ---------- */

int* degreesTab(Graph* g);

void printVertex(Vertex* v);
void printEdge(Edge* e);
void printGraph(Graph* g);

int countTriangle(Graph* g);

int getBaseIndex(const char* name);
int getSuffix(const char* name);

int vertexIndex(Graph* g, Vertex* v) ;
void removeEdgePtr(Graph* g, Edge* e);

int getRankGroup(Vertex* v);
int compareVertexName(const void* a, const void* b);
 bool shareCommonS(Graph* g, Vertex* a, Vertex* b);

/* ---------- Clonage et libération ---------- */

Graph* cloneGraph(Graph* g);
void freeGraph(Graph* g);

Graph* PathOfTriangle(int path, int core, int maxPerBox, int maxLR);

Graph* parsePathOfTriangle(const char* dotFilename);



void generateDotFile(Graph* g, const char* filename);
void generateDotFileSimplified(Graph* g, const char* filename);
void generateDotFileWithTriplets(Graph* g, const char* filename, Triplet* T);
void generateDotFileWithPairs(Graph* g, const char* filename, Pair* M);
bool edgeExists(Graph* g, Vertex* v1, Vertex* v2);


#endif /* GRAPH_H */