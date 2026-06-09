/**
 * graph.c – Implémentation d’un graphe simple avec listes d’adjacence
 * Optimisé pour les requêtes de voisinage (O(degré) au lieu de O(nbe)).
 *
 * Les sommets et les arêtes sont alloués dynamiquement et leur durée de vie
 * est gérée par le graphe.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Graph.h"
#include "Algo.h"

#define BUFFER_SIZE 256


/* ---------- Création des éléments ---------- */

Vertex* createVertex(char* name) {
    Vertex* v = malloc(sizeof(Vertex));
    v->name = malloc(strlen(name) + 1);
    strcpy(v->name, name);
    return v;
}

Edge* createEdge(char* name, Vertex* v1, Vertex* v2) {
    Edge* e = malloc(sizeof(Edge));
    e->name = malloc(strlen(name) + 1);
    strcpy(e->name, name);
    e->endpoints[0] = v1;
    e->endpoints[1] = v2;
    return e;
}

/**
 * Crée un graphe pouvant contenir au maximum nbvMAX sommets.
 * Le nombre maximal d'arêtes est déduit pour un graphe complet.
 */
Graph* createGraph(char* name, int nbvMAX) {
    Graph* g = malloc(sizeof(Graph));
    g->name = malloc(strlen(name) + 1);
    strcpy(g->name, name);
    g->nbvMAX = nbvMAX;
    g->nbv = 0;
    g->nbeMAX = (nbvMAX * (nbvMAX - 1)) / 2;
    g->nbe = 0;

    g->vertices = calloc(nbvMAX, sizeof(Vertex*));
    g->edges = calloc(g->nbeMAX, sizeof(Edge*));
    g->adj = calloc(nbvMAX, sizeof(AdjList));   // tout à zéro
    return g;
}

/* ---------- Recherche ---------- */

Vertex* getVertex(Graph* g, char* name) {
    for (int i = 0; i < g->nbv; i++) {
        if (strcmp(g->vertices[i]->name, name) == 0)
            return g->vertices[i];
    }
    return NULL;
}

Edge* getEdge(Graph* g, char* name) {
    for (int i = 0; i < g->nbe; i++) {
        if (g->edges[i] && strcmp(g->edges[i]->name, name) == 0)
            return g->edges[i];
    }
    return NULL;
}

/* ---------- Ajout ---------- */

int addVertex(Graph* g, Vertex* v) {
    if (g->nbv >= g->nbvMAX) {
        printf("WARNING: too many vertices in %s, ignoring %s\n", g->name, v->name);
        return 0;
    }
    g->vertices[g->nbv] = v;
    g->adj[g->nbv].edges = NULL;   // sera initialisé au premier ajout d'arête
    g->adj[g->nbv].size = 0;
    g->adj[g->nbv].capacity = 0;
    v->id = g->nbv;
    g->nbv++;
    return 1;
}

/* Trouve l'index d'un sommet dans le tableau vertices (comparaison de pointeurs) */
int vertexIndex(Graph* g, Vertex* v) {
    (void) g;   // pour éviter un avertissement "unused parameter"
    if (v == NULL) return -1;
    return v->id;
}

int addEdge(Graph* g, Edge* e) {
    if (g->nbe >= g->nbeMAX) {
        printf("WARNING: too many edges in %s, ignoring %s\n", g->name, e->name);
        return 0;
    }

    int i1 = vertexIndex(g, e->endpoints[0]);
    int i2 = vertexIndex(g, e->endpoints[1]);
    if (i1 == -1 || i2 == -1) {
        printf("WARNING: edge %s references unknown vertices\n", e->name);
        return 0;
    }

    g->edges[g->nbe] = e;
    g->nbe++;

    /* Ajout dans les listes d'adjacence des deux extrémités */
    for (int pass = 0; pass < 2; pass++) {
        int idx = (pass == 0) ? i1 : i2;
        AdjList* adj = &g->adj[idx];
        if (adj->size == adj->capacity) {
            int newcap = adj->capacity == 0 ? 2 : adj->capacity * 2;
            Edge** tmp = realloc(adj->edges, newcap * sizeof(Edge*));
            if (!tmp) return 0;
            adj->edges = tmp;
            adj->capacity = newcap;
        }
        adj->edges[adj->size++] = e;
    }
    return 1;
}

/* ---------- Suppression ---------- */

/* Supprime une arête à partir de son pointeur (fonction interne) */
void removeEdgePtr(Graph* g, Edge* e) {
    int i1 = vertexIndex(g, e->endpoints[0]);
    int i2 = vertexIndex(g, e->endpoints[1]);

    /* Retirer des listes d'adjacence */
    for (int pass = 0; pass < 2; pass++) {
        int idx = (pass == 0) ? i1 : i2;
        AdjList* adj = &g->adj[idx];
        for (int k = 0; k < adj->size; k++) {
            if (adj->edges[k] == e) {
                // décalage
                for (int m = k; m < adj->size - 1; m++)
                    adj->edges[m] = adj->edges[m + 1];
                adj->size--;
                break;
            }
        }
    }

    /* Retirer du tableau global g->edges */
    for (int i = 0; i < g->nbe; i++) {
        if (g->edges[i] == e) {
            for (int j = i; j < g->nbe - 1; j++)
                g->edges[j] = g->edges[j + 1];
            g->nbe--;
            break;
        }
    }

    free(e->name);
    free(e);
}

void deleteEdge(Graph* g, Edge e) {
    Edge* ptr = getEdge(g, e.name);
    if (ptr) removeEdgePtr(g, ptr);
}

void deleteVertex(Graph* g, Vertex v) {
    Vertex* ptr = getVertex(g, v.name);
    if (!ptr) return;

    int idx = vertexIndex(g, ptr);
    if (idx == -1) return;

    /* Copie de la liste d'adjacence car elle sera modifiée par removeEdgePtr */
    AdjList* adj = &g->adj[idx];
    int nb_edges = adj->size;
    Edge** edges_copy = malloc(nb_edges * sizeof(Edge*));
    memcpy(edges_copy, adj->edges, nb_edges * sizeof(Edge*));

    for (int i = 0; i < nb_edges; i++) {
        removeEdgePtr(g, edges_copy[i]);
    }
    free(edges_copy);

    /* Libération de la liste d'adjacence du sommet supprimé */
    free(adj->edges);
    adj->edges = NULL;
    adj->size = adj->capacity = 0;

    /* Décalage des tableaux vertices et adj */
    free(ptr->name);
    free(ptr);
    for (int i = idx; i < g->nbv - 1; i++) {
        g->vertices[i] = g->vertices[i + 1];
        g->adj[i] = g->adj[i + 1];
    }
    g->vertices[g->nbv - 1] = NULL;
    g->adj[g->nbv - 1].edges = NULL;
    g->adj[g->nbv - 1].size = 0;
    g->adj[g->nbv - 1].capacity = 0;

    
    for (int j = idx; j < g->nbv - 1; j++) {
        g->vertices[j]->id = j;      // remet l'id en accord avec l'index
    }

    g->nbv--;
}

/* ---------- Voisinage (optimisé) ---------- */

IssuedEdges* getIssuedEdges(Graph* g, Vertex* v) {
    IssuedEdges* neigh = malloc(sizeof(IssuedEdges));
    neigh->v = v;

    int idx = vertexIndex(g, v);
    if (idx == -1) {
        neigh->issuedEdges = NULL;
        neigh->size = 0;
        return neigh;
    }

    AdjList* adj = &g->adj[idx];
    neigh->size = adj->size;
    neigh->issuedEdges = malloc(adj->size * sizeof(Edge*));
    memcpy(neigh->issuedEdges, adj->edges, adj->size * sizeof(Edge*));
    return neigh;
}

void freeIssuedEdges(IssuedEdges* n) {
    free(n->issuedEdges);
    free(n);
}

/* ---------- Utilitaires ---------- */

int* degreesTab(Graph* g) {
    int* tab = malloc(g->nbv * sizeof(int));
    for (int i = 0; i < g->nbv; i++) {
        tab[i] = g->adj[i].size;
    }
    return tab;
}

void printVertex(Vertex* v) {
    printf("%s", v->name);
}

void printEdge(Edge* e) {
    printf("%s : %s -- %s", e->name,
           e->endpoints[0]->name, e->endpoints[1]->name);
}

void printGraph(Graph* g) {
    printf("--- Graph \"%s\" ---\n", g->name);
    printf("Vertices:\n");
    for (int i = 0; i < g->nbv; i++) {
        printf("  "); printVertex(g->vertices[i]); printf("\n");
    }
    printf("Edges:\n");
    for (int i = 0; i < g->nbe; i++) {
        printf("  "); printEdge(g->edges[i]); printf("\n");
    }
    printf("Adjacency lists:\n");
    for (int i = 0; i < g->nbv; i++) {
        printf("  %s -> ", g->vertices[i]->name);
        for (int j = 0; j < g->adj[i].size; j++) {
            printf("%s ", g->adj[i].edges[j]->name);
        }
        printf("\n");
    }
    printf("--------\n");
}

/* ---------- Clonage ---------- */

Graph* cloneGraph(Graph* g) {
    char buff[BUFFER_SIZE];
    snprintf(buff, BUFFER_SIZE, "%s_clone", g->name);
    Graph* clone = createGraph(buff, g->nbvMAX);

    // Tableau de correspondance ancien -> nouveau sommet
    Vertex** map = malloc(g->nbvMAX * sizeof(Vertex*));

    // Cloner les sommets
    for (int i = 0; i < g->nbv; i++) {
        Vertex* new_v = createVertex(g->vertices[i]->name);
        addVertex(clone, new_v);
        map[i] = new_v;
    }

    // Cloner les arêtes en utilisant les nouveaux sommets
    for (int i = 0; i < g->nbe; i++) {
        Edge* old_e = g->edges[i];
        Vertex* new_v1 = map[vertexIndex(g, old_e->endpoints[0])];
        Vertex* new_v2 = map[vertexIndex(g, old_e->endpoints[1])];
        Edge* new_e = createEdge(old_e->name, new_v1, new_v2);
        addEdge(clone, new_e);
    }

    free(map);
    return clone;
}

/* ---------- Libération ---------- */

void freeGraph(Graph* g) {
    if (!g) return;

    for (int i = 0; i < g->nbv; i++) {
        free(g->vertices[i]->name);
        free(g->vertices[i]);
        free(g->adj[i].edges);
    }
    free(g->vertices);
    free(g->adj);

    for (int i = 0; i < g->nbe; i++) {
        free(g->edges[i]->name);
        free(g->edges[i]);
    }
    free(g->edges);

    free(g->name);
    free(g);
}

/*------Utils-------------*/


/**
 * Compte le nombre de triangles dans le graphe g.
 */
int countTriangle(Graph* g) {
    int n = g->nbv;
    if (n < 3) return 0;

    // Tableau de marquage (un entier par sommet) et compteur de “temps”
    int* mark = calloc(n, sizeof(int));
    int cur_mark = 0;
    int triangles = 0;

    // Pour chaque sommet u (indice i)
    for (int i = 0; i < n; i++) {
        Vertex* u = g->vertices[i];
        cur_mark++;                     // nouvelle marque
        // Marquer tous les voisins de u
        AdjList* adj_u = &g->adj[i];
        for (int k = 0; k < adj_u->size; k++) {
            Edge* e = adj_u->edges[k];
            Vertex* neigh = (e->endpoints[0] == u) ? e->endpoints[1] : e->endpoints[0];
            // Trouver l’indice du voisin (on peut optimiser en stockant l’indice dans Vertex)
            int idx = -1;
            for (int j = 0; j < n; j++) {
                if (g->vertices[j] == neigh) { idx = j; break; }
            }
            if (idx != -1) mark[idx] = cur_mark;
        }

        // Pour chaque voisin v de u d’indice supérieur à i
        for (int k = 0; k < adj_u->size; k++) {
            Edge* e = adj_u->edges[k];
            Vertex* v = (e->endpoints[0] == u) ? e->endpoints[1] : e->endpoints[0];
            int idx_v = -1;
            for (int j = 0; j < n; j++) {
                if (g->vertices[j] == v) { idx_v = j; break; }
            }
            if (idx_v <= i) continue;   // ne considérer que les v > i

            // Chercher les voisins w de v avec w > idx_v et marqués
            AdjList* adj_v = &g->adj[idx_v];
            for (int k2 = 0; k2 < adj_v->size; k2++) {
                Edge* e2 = adj_v->edges[k2];
                Vertex* w = (e2->endpoints[0] == v) ? e2->endpoints[1] : e2->endpoints[0];
                int idx_w = -1;
                for (int j = 0; j < n; j++) {
                    if (g->vertices[j] == w) { idx_w = j; break; }
                }
                if (idx_w > idx_v && mark[idx_w] == cur_mark) {
                    triangles++;
                }
            }
        }
    }

    free(mark);
    return triangles;
}
/* ---------- Exportation en dotfile ---------- */

/**
 * Génère un fichier .dot représentant le graphe g.
 * @param g        Graphe à exporter
 * @param filename Nom du fichier de sortie (ex: "mon_graphe.dot")

 */
#include <string.h>

/* Retourne l’indice de base d’un sommet (le premier entier après la lettre) */
int getBaseIndex(const char* name) {
    int idx = 0;
    sscanf(name, "%*c%d", &idx);  // saute le premier caractère (s ou m)
    return idx;
}

/* Retourne le suffixe d’un sommet (0,1,2,3 pour m*_j ; 0,1,2 pour s*_j ; -1 si pas de suffixe) */
int getSuffix(const char* name) {
    const char* underscore = strchr(name, '_');
    if (underscore) {
        int suf = 0;
        sscanf(underscore + 1, "%d", &suf);
        return suf;
    }
    return -1;
}

/* Détermine un identifiant de groupe pour le rank=same.
   L’ordre souhaité (bas → haut) : s simples, m_0, m_1, m_2, m_3, s_0, s_1, s_2, ...
   On retourne un entier représentant la priorité (plus petit = plus bas). */
int getRankGroup(Vertex* v) {
    const char* name = v->name;
    if (name[0] == 's' && !strchr(name, '_'))
        return 0;                      // s2, s4, … tout en bas
    if (name[0] == 'm')
        return 1 + getSuffix(name);    // m_0 -> 1, m_1 -> 2, m_2 -> 3, m_3 -> 4
    if (name[0] == 's')
        return 10 + getSuffix(name);   // s_0 -> 10, s_1 -> 11, s_2 -> 12 … (au‑dessus des m)
    return 99;
}

/* Comparaison pour trier les sommets d’un même groupe par ordre alphabétique (optionnel) */
int compareVertexName(const void* a, const void* b) {
    Vertex* va = *(Vertex**)a;
    Vertex* vb = *(Vertex**)b;
    return strcmp(va->name, vb->name);
}
// Tri par ordre de hauteur (suffixe ou type) pour aligner du bas vers le haut
// On définit un ordre : s simple (sans suffixe) < s_0 < s_1 < s_2 ; m_0 < m_1 < m_2 < m_3
int vertexOrder(const void* a, const void* b) {
    Vertex* va = *(Vertex**)a;
    Vertex* vb = *(Vertex**)b;
    int rankA = getRankGroup(va);
    int rankB = getRankGroup(vb);
    if (rankA != rankB) return rankA - rankB;  // du plus bas au plus haut
    // En cas d'égalité (ne devrait pas arriver dans une même colonne), ordre alphabétique
    return strcmp(va->name, vb->name);
}
/**
 * Génère un fichier DOT avec :
 * - rangs automatiques selon le type du sommet,
 * - alignement vertical par colonne (arêtes invisibles).
 * @param g        Graphe à exporter
 * @param filename Nom du fichier .dot de sortie
 */
void generateDotFile(Graph* g, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Erreur d'ouverture de %s\n", filename);
        return;
    }

    fprintf(f, "graph \"%s\" {\n", g->name);
    fprintf(f, "\tlayout=dot;\n");
    fprintf(f, "\trankdir=TB;\n");
    fprintf(f, "\tsplines=false;\n");
    fprintf(f, "\tnewrank=true;\n");

    // ----------------------------------------------------------------------
    // 1. Regrouper les sommets par groupe de rang (rank=same)
    // ----------------------------------------------------------------------
    // On utilise des listes temporaires : un tableau de tableaux pour chaque groupe
    #define MAX_GROUP 50
    Vertex** groups[MAX_GROUP] = {NULL};
    int       groupSize[MAX_GROUP] = {0};

    for (int i = 0; i < g->nbv; i++) {
        int grp = getRankGroup(g->vertices[i]);
        if (grp < 0 || grp >= MAX_GROUP) continue;
        groups[grp] = realloc(groups[grp], (groupSize[grp] + 1) * sizeof(Vertex*));
        groups[grp][groupSize[grp]++] = g->vertices[i];
    }

    // Trier chaque groupe par nom pour une sortie propre
    for (int grp = 0; grp < MAX_GROUP; grp++) {
        if (groupSize[grp] > 1)
            qsort(groups[grp], groupSize[grp], sizeof(Vertex*), compareVertexName);
    }

    // Écrire les blocs rank=same
    for (int grp = 0; grp < MAX_GROUP; grp++) {
        if (groupSize[grp] == 0) continue;
        fprintf(f, "\t{ rank=same;");
        for (int k = 0; k < groupSize[grp]; k++) {
            fprintf(f, " %s;", groups[grp][k]->name);
        }
        fprintf(f, " }\n");
    }

    // ----------------------------------------------------------------------
    // 2. Déclaration des sommets (shape)
    // ----------------------------------------------------------------------
    fprintf(f, "\n");
    for (int i = 0; i < g->nbv; i++) {
        fprintf(f, "\t%s [shape=circle];\n", g->vertices[i]->name);
    }

    // ----------------------------------------------------------------------
    // 3. Arêtes réelles
    // ----------------------------------------------------------------------
    fprintf(f, "\n");
    for (int i = 0; i < g->nbe; i++) {
        Edge* e = g->edges[i];
        fprintf(f, "\t%s -- %s;\n", e->endpoints[0]->name, e->endpoints[1]->name);
    }

    // ----------------------------------------------------------------------
    // 4. Arêtes invisibles pour l’alignement vertical par colonne
    // ----------------------------------------------------------------------
    // Pour chaque indice de base (pair → s, impair → m), on collecte tous les
    // sommets de cet indice, puis on les trie par ordre de suffixe décroissant
    // (pour que le plus grand suffixe soit en haut) et on les relie.
    fprintf(f, "\n\t// Contraintes d'alignement vertical par groupe\n");

    // Tableau pour collecter les sommets par indice de base
    #define MAX_BASE 100
    Vertex** baseLists[MAX_BASE] = {NULL};
    int baseCount[MAX_BASE] = {0};

    for (int i = 0; i < g->nbv; i++) {
        int base = getBaseIndex(g->vertices[i]->name);
        if (base >= 0 && base < MAX_BASE) {
            baseLists[base] = realloc(baseLists[base], (baseCount[base] + 1) * sizeof(Vertex*));
            baseLists[base][baseCount[base]++] = g->vertices[i];
        }
    }

    

    for (int base = 0; base < MAX_BASE; base++) {
        if (baseCount[base] <= 1) continue;
        qsort(baseLists[base], baseCount[base], sizeof(Vertex*), vertexOrder);

        // Relier dans l'ordre : le plus haut (dernier) vers le plus bas (premier)
        for (int k = baseCount[base] - 1; k > 0; k--) {
            fprintf(f, "\t%s -- %s [style=invis];\n",
                    baseLists[base][k]->name, baseLists[base][k-1]->name);
        }
    }

    // ----------------------------------------------------------------------
    // Libération des tableaux temporaires
    // ----------------------------------------------------------------------
    for (int grp = 0; grp < MAX_GROUP; grp++) free(groups[grp]);
    for (int base = 0; base < MAX_BASE; base++) free(baseLists[base]);

    fprintf(f, "}\n");
    fclose(f);
    printf("Fichier DOT généré : %s\n", filename);
}


void generateDotFileSimplified(Graph* g, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Erreur d'ouverture de %s\n", filename);
        return;
    }

    fprintf(f, "graph \"%s\" {\n", g->name);
    fprintf(f, "\tlayout=dot;\n");
    fprintf(f, "\trankdir=TB;\n");
    fprintf(f, "\tsplines=false;\n");
    fprintf(f, "\tnewrank=true;\n");

    // ----------------------------------------------------------------------
    // 1. Regrouper les sommets par groupe de rang (rank=same)
    // ----------------------------------------------------------------------
    #define MAX_GROUP 50
    Vertex** groups[MAX_GROUP] = {NULL};
    int       groupSize[MAX_GROUP] = {0};

    for (int i = 0; i < g->nbv; i++) {
        int grp = getRankGroup(g->vertices[i]);
        if (grp < 0 || grp >= MAX_GROUP) continue;
        groups[grp] = realloc(groups[grp], (groupSize[grp] + 1) * sizeof(Vertex*));
        groups[grp][groupSize[grp]++] = g->vertices[i];
    }

    // Trier chaque groupe par nom pour une sortie propre
    for (int grp = 0; grp < MAX_GROUP; grp++) {
        if (groupSize[grp] > 1)
            qsort(groups[grp], groupSize[grp], sizeof(Vertex*), compareVertexName);
    }

    // Écrire les blocs rank=same
    for (int grp = 0; grp < MAX_GROUP; grp++) {
        if (groupSize[grp] == 0) continue;
        fprintf(f, "\t{ rank=same;");
        for (int k = 0; k < groupSize[grp]; k++) {
            fprintf(f, " %s;", groups[grp][k]->name);
        }
        fprintf(f, " }\n");
    }

    // ----------------------------------------------------------------------
    // 2. Déclaration des sommets (shape)
    // ----------------------------------------------------------------------
    fprintf(f, "\n");
    for (int i = 0; i < g->nbv; i++) {
        fprintf(f, "\t%s [shape=circle];\n", g->vertices[i]->name);
    }

    // ----------------------------------------------------------------------
    // 3. Arêtes réelles (uniquement entre colonnes d'indices consécutifs)
    // ----------------------------------------------------------------------
    fprintf(f, "\n");
    for (int i = 0; i < g->nbe; i++) {
        Edge* e = g->edges[i];
        int base1 = getBaseIndex(e->endpoints[0]->name);
        int base2 = getBaseIndex(e->endpoints[1]->name);
        if (abs(base1 - base2) <= 2) {
            fprintf(f, "\t%s -- %s;\n", e->endpoints[0]->name, e->endpoints[1]->name);
        }
    }

    // ----------------------------------------------------------------------
    // 4. Arêtes invisibles pour l’alignement vertical par colonne
    // ----------------------------------------------------------------------
    fprintf(f, "\n\t// Contraintes d'alignement vertical par groupe\n");

    #define MAX_BASE 100
    Vertex** baseLists[MAX_BASE] = {NULL};
    int baseCount[MAX_BASE] = {0};

    for (int i = 0; i < g->nbv; i++) {
        int base = getBaseIndex(g->vertices[i]->name);
        if (base >= 0 && base < MAX_BASE) {
            baseLists[base] = realloc(baseLists[base], (baseCount[base] + 1) * sizeof(Vertex*));
            baseLists[base][baseCount[base]++] = g->vertices[i];
        }
    }

    for (int base = 0; base < MAX_BASE; base++) {
        if (baseCount[base] <= 1) continue;
        qsort(baseLists[base], baseCount[base], sizeof(Vertex*), vertexOrder);

        for (int k = baseCount[base] - 1; k > 0; k--) {
            fprintf(f, "\t%s -- %s [style=invis];\n",
                    baseLists[base][k]->name, baseLists[base][k-1]->name);
        }
    }

    // ----------------------------------------------------------------------
    // Libération des tableaux temporaires
    // ----------------------------------------------------------------------
    for (int grp = 0; grp < MAX_GROUP; grp++) free(groups[grp]);
    for (int base = 0; base < MAX_BASE; base++) free(baseLists[base]);

    fprintf(f, "}\n");
    fclose(f);
    printf("Fichier DOT généré : %s\n", filename);
}




/**
 * Génère un fichier .dot pour le graphe g, en colorant en rouge les arêtes
 * qui appartiennent à au moins un triplet de la liste T.
 * Les autres arêtes sont en noir. Seules les arêtes reliant deux colonnes
 * d'indices consécutifs sont affichées.
 *
 * @param g        Graphe à visualiser
 * @param filename Nom du fichier .dot de sortie
 * @param T        Liste chaînée de triplets (peut être NULL)
 */
void generateDotFileWithTriplets(Graph* g, const char* filename, Triplet* T) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Erreur d'ouverture de %s\n", filename);
        return;
    }

    int n = g->nbv;

    // Construction d'une matrice booléenne pour marquer les paires de triplets
    bool** in_triplet = malloc(n * sizeof(bool*));
    for (int i = 0; i < n; i++) {
        in_triplet[i] = calloc(n, sizeof(bool));
    }

    // Remplir la matrice avec les triplets
    for (Triplet* t = T; t != NULL; t = t->next) {
        int a = t->v[0]->id;
        int b = t->v[1]->id;
        int c = t->v[2]->id;
        in_triplet[a][b] = in_triplet[b][a] = true;
        in_triplet[a][c] = in_triplet[c][a] = true;
        in_triplet[b][c] = in_triplet[c][b] = true;
    }

    // ----- En-tête du DOT -----
    fprintf(f, "graph \"%s\" {\n", g->name);
    fprintf(f, "\tlayout=dot;\n");
    fprintf(f, "\trankdir=TB;\n");
    fprintf(f, "\tsplines=false;\n");
    fprintf(f, "\tnewrank=true;\n");

    // ----- 1. Blocs rank=same (comme avant) -----
    #define MAX_GROUP 50
    Vertex** groups[MAX_GROUP] = {NULL};
    int groupSize[MAX_GROUP] = {0};

    for (int i = 0; i < n; i++) {
        int grp = getRankGroup(g->vertices[i]);
        if (grp < 0 || grp >= MAX_GROUP) continue;
        groups[grp] = realloc(groups[grp], (groupSize[grp] + 1) * sizeof(Vertex*));
        groups[grp][groupSize[grp]++] = g->vertices[i];
    }

    for (int grp = 0; grp < MAX_GROUP; grp++) {
        if (groupSize[grp] > 1)
            qsort(groups[grp], groupSize[grp], sizeof(Vertex*), compareVertexName);
    }

    for (int grp = 0; grp < MAX_GROUP; grp++) {
        if (groupSize[grp] == 0) continue;
        fprintf(f, "\t{ rank=same;");
        for (int k = 0; k < groupSize[grp]; k++) {
            fprintf(f, " %s;", groups[grp][k]->name);
        }
        fprintf(f, " }\n");
    }

    // ----- 2. Déclaration des sommets -----
    fprintf(f, "\n");
    for (int i = 0; i < n; i++) {
        fprintf(f, "\t%s [shape=circle];\n", g->vertices[i]->name);
    }

    // ----- 3. Arêtes réelles (indices consécutifs) avec couleur rouge si triplet -----
    fprintf(f, "\n");
    for (int i = 0; i < g->nbe; i++) {
        Edge* e = g->edges[i];
        int base1 = getBaseIndex(e->endpoints[0]->name);
        int base2 = getBaseIndex(e->endpoints[1]->name);
        if (abs(base1 - base2) <=2) {
            int u = e->endpoints[0]->id;
            int v = e->endpoints[1]->id;
            if (in_triplet[u][v]) {
                fprintf(f, "\t%s -- %s [color=red];\n", e->endpoints[0]->name, e->endpoints[1]->name);
            } else {
                fprintf(f, "\t%s -- %s;\n", e->endpoints[0]->name, e->endpoints[1]->name);
            }
        }
    }

    // ----- 4. Contraintes d'alignement vertical (invisibles) -----
    fprintf(f, "\n\t// Contraintes d'alignement vertical par groupe\n");

    #define MAX_BASE 100
    Vertex** baseLists[MAX_BASE] = {NULL};
    int baseCount[MAX_BASE] = {0};

    for (int i = 0; i < n; i++) {
        int base = getBaseIndex(g->vertices[i]->name);
        if (base >= 0 && base < MAX_BASE) {
            baseLists[base] = realloc(baseLists[base], (baseCount[base] + 1) * sizeof(Vertex*));
            baseLists[base][baseCount[base]++] = g->vertices[i];
        }
    }

    for (int base = 0; base < MAX_BASE; base++) {
        if (baseCount[base] <= 1) continue;
        qsort(baseLists[base], baseCount[base], sizeof(Vertex*), vertexOrder);
        for (int k = baseCount[base] - 1; k > 0; k--) {
            fprintf(f, "\t%s -- %s [style=invis];\n",
                    baseLists[base][k]->name, baseLists[base][k-1]->name);
        }
    }

    // ----- Libération -----
    for (int grp = 0; grp < MAX_GROUP; grp++) free(groups[grp]);
    for (int base = 0; base < MAX_BASE; base++) free(baseLists[base]);

    for (int i = 0; i < n; i++) free(in_triplet[i]);
    free(in_triplet);

    fprintf(f, "}\n");
    fclose(f);
    printf("Fichier DOT généré : %s\n", filename);
}


/**
 * Génère un fichier DOT en colorant en rouge les arêtes qui appartiennent
 * à la liste de paires M (le couplage). Les autres arêtes sont en noir.
 * Seules les arêtes entre colonnes d'indices de base suffisamment proches
 * (écart ≤ 2) sont affichées.
 *
 * @param g        Graphe à visualiser
 * @param filename Nom du fichier .dot de sortie
 * @param M        Liste chaînée de paires (couplage), peut être NULL
 */
void generateDotFileWithPairs(Graph* g, const char* filename, Pair* M) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Erreur d'ouverture de %s\n", filename);
        return;
    }

    int n = g->nbv;

    // Construction d'une matrice booléenne pour marquer les arêtes du couplage
    bool** in_matching = malloc(n * sizeof(bool*));
    for (int i = 0; i < n; i++) {
        in_matching[i] = calloc(n, sizeof(bool));
    }
    
    // Remplir la matrice avec les paires M
    for (Pair* p = M; p != NULL; p = p->next) {
        int a = p->v[0]->id;
        int b = p->v[1]->id;
        //printf("%d,%d\n",a,b);
        in_matching[a][b] = in_matching[b][a] = true;
        
    }
    
    // ----- En-tête du DOT -----
    fprintf(f, "graph \"%s\" {\n", g->name);
    fprintf(f, "\tlayout=dot;\n");
    fprintf(f, "\trankdir=TB;\n");
    fprintf(f, "\tsplines=false;\n");
    fprintf(f, "\tnewrank=true;\n");

    // ----- 1. Blocs rank=same (identiques à la version précédente) -----
    #define MAX_GROUP 50
    Vertex** groups[MAX_GROUP] = {NULL};
    int groupSize[MAX_GROUP] = {0};

    for (int i = 0; i < n; i++) {
        int grp = getRankGroup(g->vertices[i]);
        if (grp < 0 || grp >= MAX_GROUP) continue;
        groups[grp] = realloc(groups[grp], (groupSize[grp] + 1) * sizeof(Vertex*));
        groups[grp][groupSize[grp]++] = g->vertices[i];
    }

    for (int grp = 0; grp < MAX_GROUP; grp++) {
        if (groupSize[grp] > 1)
            qsort(groups[grp], groupSize[grp], sizeof(Vertex*), compareVertexName);
    }

    for (int grp = 0; grp < MAX_GROUP; grp++) {
        if (groupSize[grp] == 0) continue;
        fprintf(f, "\t{ rank=same;");
        for (int k = 0; k < groupSize[grp]; k++) {
            fprintf(f, " %s;", groups[grp][k]->name);
        }
        fprintf(f, " }\n");
    }
    
    // ----- 2. Déclaration des sommets -----
    fprintf(f, "\n");
    for (int i = 0; i < n; i++) {
        fprintf(f, "\t%s [shape=circle];\n", g->vertices[i]->name);
    }

    // ----- 3. Arêtes réelles avec couleur rouge si dans M -----
    fprintf(f, "\n");
    for (int i = 0; i < g->nbe; i++) {
    Edge* e = g->edges[i];
    int base1 = getBaseIndex(e->endpoints[0]->name);
    int base2 = getBaseIndex(e->endpoints[1]->name);
    int u = e->endpoints[0]->id;
    int v = e->endpoints[1]->id;
    int in_match = in_matching[u][v];   // 1 si l'arête est dans M, 0 sinon

    // Afficher si l'écart est ≤ 2, OU si l'arête fait partie du matching
    if (abs(base1 - base2) <= 2 || in_match) {
        if (in_match) {
            fprintf(f, "\t%s -- %s [color=red];\n",
                    e->endpoints[0]->name, e->endpoints[1]->name);
        } else {
            fprintf(f, "\t%s -- %s;\n",
                    e->endpoints[0]->name, e->endpoints[1]->name);
        }
    }
}

    // ----- 4. Contraintes d'alignement vertical (invisibles) -----
    fprintf(f, "\n\t// Contraintes d'alignement vertical par groupe\n");

    #define MAX_BASE 100
    Vertex** baseLists[MAX_BASE] = {NULL};
    int baseCount[MAX_BASE] = {0};

    for (int i = 0; i < n; i++) {
        int base = getBaseIndex(g->vertices[i]->name);
        if (base >= 0 && base < MAX_BASE) {
            baseLists[base] = realloc(baseLists[base], (baseCount[base] + 1) * sizeof(Vertex*));
            baseLists[base][baseCount[base]++] = g->vertices[i];
        }
    }

    for (int base = 0; base < MAX_BASE; base++) {
        if (baseCount[base] <= 1) continue;
        qsort(baseLists[base], baseCount[base], sizeof(Vertex*), vertexOrder);
        for (int k = baseCount[base] - 1; k > 0; k--) {
            fprintf(f, "\t%s -- %s [style=invis];\n",
                    baseLists[base][k]->name, baseLists[base][k-1]->name);
        }
    }

    // ----- Libération -----
    for (int grp = 0; grp < MAX_GROUP; grp++) free(groups[grp]);
    for (int base = 0; base < MAX_BASE; base++) free(baseLists[base]);

    for (int i = 0; i < n; i++) free(in_matching[i]);
    free(in_matching);

    fprintf(f, "}\n");
    fclose(f);
    printf("Fichier DOT généré : %s\n", filename);
}


// Fonction interne : vrai si une arête existe déjà entre v1 et v2
bool edgeExists(Graph* g, Vertex* v1, Vertex* v2) {
    for (int i = 0; i < g->nbe; i++) {
        Edge* e = g->edges[i];
        if ((e->endpoints[0] == v1 && e->endpoints[1] == v2) ||
            (e->endpoints[0] == v2 && e->endpoints[1] == v1))
            return true;
        }
        return false;
    }


    /**
 * Renvoie true si a et b ont au moins un voisin commun dont le nom commence par 's'.
 * Utilise les listes d'adjacence du graphe.
 */
 bool shareCommonS(Graph* g, Vertex* a, Vertex* b) {
    // Retrouver l'indice des sommets dans g->vertices
    int idxA = -1, idxB = -1;
    for (int i = 0; i < g->nbv; i++) {
        if (g->vertices[i] == a) idxA = i;
        if (g->vertices[i] == b) idxB = i;
    }
    if (idxA == -1 || idxB == -1) return false;

    // Marquer les voisins de type 's' de a
    bool* marked = calloc(g->nbv, sizeof(bool));
    for (int i = 0; i < g->adj[idxA].size; i++) {
        Edge* e = g->adj[idxA].edges[i];
        Vertex* neigh = (e->endpoints[0] == a) ? e->endpoints[1] : e->endpoints[0];
        if (neigh->name[0] == 's') {
            int idxN = -1;
            for (int j = 0; j < g->nbv; j++) {
                if (g->vertices[j] == neigh) { idxN = j; break; }
            }
            if (idxN != -1) marked[idxN] = true;
        }
    }

    // Vérifier si b a un voisin marqué
    for (int i = 0; i < g->adj[idxB].size; i++) {
        Edge* e = g->adj[idxB].edges[i];
        Vertex* neigh = (e->endpoints[0] == b) ? e->endpoints[1] : e->endpoints[0];
        if (neigh->name[0] == 's') {
            int idxN = -1;
            for (int j = 0; j < g->nbv; j++) {
                if (g->vertices[j] == neigh) { idxN = j; break; }
            }
            if (idxN != -1 && marked[idxN]) {
                free(marked);
                return true;
            }
        }
    }
    free(marked);
    return false;
}
/* ---------- PathOfTriangle ---------- */
Graph* PathOfTriangle(int path, int core, int maxPerBox, int maxLR) {
    int nbVertices = 2 * maxPerBox * path + 2 * maxLR * (path);
    Graph* g = createGraph("PoT", nbVertices);
    char name[BUFFER_SIZE];

    // Tableau des sommets principaux du chemin (indices pairs 2,4,...,2*path)
    Vertex** pathVertices = malloc(path * sizeof(Vertex*));

    // Compteur d'arêtes entre paires d'indices pairs consécutifs
    int* edgeCounters = calloc(path - 1, sizeof(int));

    // --- Création du chemin principal (s2, s4, ..., s(2*path)) ---
    for (int i = 0; i < path; i++) {
        int idx = 2 * (i + 1);
        snprintf(name, BUFFER_SIZE, "s%d", idx);
        pathVertices[i] = createVertex(name);
        addVertex(g, pathVertices[i]);
    }

    // Arêtes du chemin et leurs sommets M
    for (int i = 0; i < path - 1; i++) {
        snprintf(name, BUFFER_SIZE, "e_s%d_s%d", 2*(i+1), 2*(i+2));
        addEdge(g, createEdge(name, pathVertices[i], pathVertices[i+1]));

        int left  = 2 * (i + 1);
        int right = 2 * (i + 2);
        int mid   = left + 1;
        int cnt   = edgeCounters[i]++;
        snprintf(name, BUFFER_SIZE, "m%d_%d", mid, cnt);
        Vertex* m = createVertex(name);
        addVertex(g, m);

        snprintf(name, BUFFER_SIZE, "e_m%d_%d_s%d", mid, cnt, left);
        addEdge(g, createEdge(name, m, pathVertices[i]));
        snprintf(name, BUFFER_SIZE, "e_m%d_%d_s%d", mid, cnt, right);
        addEdge(g, createEdge(name, m, pathVertices[i+1]));
    }

    // --- Boîtes (copies) sur les sommets intérieurs ---
    bool* multiplied = calloc(path, sizeof(bool)); // true si le sommet a été multiplié
    bool previousTaken = false;
    for (int i = 1; i < path - 1; i++) {
        if (!previousTaken && rand() % 2) {
            multiplied[i] = true;
            int k = rand() % maxPerBox;
            for (int j = 0; j < k; j++) {
                int baseIdx = 2 * (i + 1);
                snprintf(name, BUFFER_SIZE, "s%d_%d", baseIdx, j);
                Vertex* new_v = createVertex(name);
                addVertex(g, new_v);

                // Connexion gauche
                snprintf(name, BUFFER_SIZE, "e_%s_s%d", new_v->name, baseIdx - 2);
                addEdge(g, createEdge(name, new_v, pathVertices[i-1]));

                int left  = baseIdx - 2;
                int right = baseIdx;
                int mid   = left + 1;
                int pair_idx = i - 1;
                int cnt = edgeCounters[pair_idx]++;
                snprintf(name, BUFFER_SIZE, "m%d_%d", mid, cnt);
                Vertex* m_left = createVertex(name);
                addVertex(g, m_left);
                snprintf(name, BUFFER_SIZE, "e_%s_s%d", m_left->name, left);
                addEdge(g, createEdge(name, m_left, pathVertices[i-1]));
                snprintf(name, BUFFER_SIZE, "e_%s_%s", m_left->name, new_v->name);
                addEdge(g, createEdge(name, m_left, new_v));

                // Connexion droite
                snprintf(name, BUFFER_SIZE, "e_%s_s%d", new_v->name, baseIdx + 2);
                addEdge(g, createEdge(name, new_v, pathVertices[i+1]));

                left  = baseIdx;
                right = baseIdx + 2;
                mid   = left + 1;
                pair_idx = i;
                cnt = edgeCounters[pair_idx]++;
                snprintf(name, BUFFER_SIZE, "m%d_%d", mid, cnt);
                Vertex* m_right = createVertex(name);
                addVertex(g, m_right);
                snprintf(name, BUFFER_SIZE, "e_%s_%s", m_right->name, new_v->name);
                addEdge(g, createEdge(name, m_right, new_v));
                snprintf(name, BUFFER_SIZE, "e_%s_s%d", m_right->name, right);
                addEdge(g, createEdge(name, m_right, pathVertices[i+1]));
            }
            previousTaken = true;
        } else {
            previousTaken = false;
        }
    }

    // --- Marquage des sommets non multipliés (ajout d'un 'n') ---
    for (int i = 0; i < path; i++) {
        if (!multiplied[i]) {
            Vertex* v = pathVertices[i];
            char* newName = malloc(strlen(v->name) + 2);
            strcpy(newName, v->name);
            strcat(newName, "n");
            free(v->name);
            v->name = newName;
        }
    }

    // --- Création des paires L/R pour les sommets marqués ---
    // On enregistre les pointeurs pour l'étape suivante
    Vertex*** L_pairs = malloc(path * sizeof(Vertex**));  // L_pairs[i][j]
    Vertex*** R_pairs = malloc(path * sizeof(Vertex**));
    int* nbLR_list = calloc(path, sizeof(int));           // nombre réel de paires par sommet
    if (core){ maxLR=1;}
    for (int i = 0; i < path; i++) {
        if (!multiplied[i]) {   // sommet marqué
            int baseIdx = 2 * (i + 1);
            int nbLR = rand() % (maxLR + 1);   // 0 .. maxLR

            if (path == 1) {
                // Cas particulier : un seul sommet → au moins 2 triangles
                nbLR = (nbLR < 2) ? 2 : nbLR;
            } else {
                // Premier sommet : si le voisin droit n'a pas de boîte, forcer ≥ 1
                if (i == 0) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "s%d_0", baseIdx + 2);
                    if (getVertex(g, buf) == NULL) {
                        nbLR = (nbLR < 1) ? 1 : nbLR;
                    }
                }
                // Dernier sommet : si le voisin gauche n'a pas de boîte, forcer ≥ 1
                else if (i == path - 1) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "s%d_0", baseIdx - 2);
                    if (getVertex(g, buf) == NULL) {
                        nbLR = (nbLR < 1) ? 1 : nbLR;
                    }
                }
            }
            
            //A verifier : conditions bizarres
            if (nbLR > maxLR) {
                fprintf(stderr, "Attention : nbLR=%d dépasse maxLR=%d pour le sommet %d\n",
                        nbLR, maxLR, i);
                // On pourrait réduire à maxLR, mais cela violerait les contraintes.
                // Il est préférable de garantir que maxLR ≥ 2 pour path==1.
                nbLR = maxLR;   // ou laisser la contrainte non satisfaite ?
            }

            nbLR_list[i] = nbLR;
            if (nbLR > 0) {
                L_pairs[i] = malloc(nbLR * sizeof(Vertex*));
                R_pairs[i] = malloc(nbLR * sizeof(Vertex*));
            } else {
                L_pairs[i] = NULL;
                R_pairs[i] = NULL;
            }

            for (int j = 0; j < nbLR; j++) {
                int lIdx = baseIdx + 1;   // indice de L
                int rIdx = baseIdx - 1;   // indice de R

                // Créer L
                snprintf(name, BUFFER_SIZE, "l%d_%d", lIdx, j);
                Vertex* L = createVertex(name);
                addVertex(g, L);
                L_pairs[i][j] = L;

                // Créer R
                snprintf(name, BUFFER_SIZE, "r%d_%d", rIdx, j);
                Vertex* R = createVertex(name);
                addVertex(g, R);
                R_pairs[i][j] = R;

                // Arêtes L -- s_i, R -- s_i, L -- R
                snprintf(name, BUFFER_SIZE, "e_%s_%s", L->name, pathVertices[i]->name);
                addEdge(g, createEdge(name, L, pathVertices[i]));
                snprintf(name, BUFFER_SIZE, "e_%s_%s", R->name, pathVertices[i]->name);
                addEdge(g, createEdge(name, R, pathVertices[i]));
                snprintf(name, BUFFER_SIZE, "e_%s_%s", L->name, R->name);
                addEdge(g, createEdge(name, L, R));
            }
        } else {
            nbLR_list[i] = 0;
            L_pairs[i] = NULL;
            R_pairs[i] = NULL;
        }
    }

 
    // --- Nouveaux sommets négatifs pour les sommets marqués ---
    if (!core){
        for (int i = 0; i < path; i++) {
            int baseIdx = 2 * (i + 1);
            int nbNew = rand() % maxPerBox;       // 0 .. maxPerBox-1
            for (int j = 0; j < nbNew; j++) {
                // Créer le sommet négatif
                snprintf(name, BUFFER_SIZE, "s%d_1_%d", baseIdx, j + 1);
                Vertex* neg = createVertex(name);
                addVertex(g, neg);
                
                // Pour chaque paire L/R existante, connecter à l'un des deux
                for (int k = 0; k < nbLR_list[i]; k++) {
                    Vertex* chosen = (rand() % 2) ? L_pairs[i][k] : R_pairs[i][k];
                    snprintf(name, BUFFER_SIZE, "e_%s_%s", neg->name, chosen->name);
                    addEdge(g, createEdge(name, neg, chosen));
                }
                    
            }
            
        }
    }


    // ON COMPTE ICI LES TRIANGLES ET ON VERIFIERA QUE CE NOMBRE NA PAS CHANGE

    int nbTriangles =countTriangle(g);
    //Graph* gSimple=cloneGraph(g);


    // ----------------------------------------------------------------------
// Relier les colonnes paires consécutives (i et i+2, i pair)
// sans créer d'arêtes multiples
// ----------------------------------------------------------------------



    for (int i = 2; i <= 2 * path; i += 2) {
        int j = i + 2;
        if (j > 2 * path) continue;   // pas de colonne voisine à droite

        // Collecter les sommets de la colonne i
        Vertex** col_i = NULL;
        int cnt_i = 0;
        // Collecter les sommets de la colonne j
        Vertex** col_j = NULL;
        int cnt_j = 0;

        for (int v = 0; v < g->nbv; v++) {
            Vertex* vert = g->vertices[v];
            if (vert->name[0] != 's') continue;          // on ne prend que les "s..."
            int base = getBaseIndex(vert->name);
            if (base == i) {
                col_i = realloc(col_i, (cnt_i + 1) * sizeof(Vertex*));
                col_i[cnt_i++] = vert;
            } else if (base == j) {
                col_j = realloc(col_j, (cnt_j + 1) * sizeof(Vertex*));
                col_j[cnt_j++] = vert;
            }
        }

        // Ajouter les arêtes manquantes entre les deux colonnes
        for (int a = 0; a < cnt_i; a++) {
            for (int b = 0; b < cnt_j; b++) {
                if (!edgeExists(g, col_i[a], col_j[b])) {
                    snprintf(name, BUFFER_SIZE, "e_%s_%s", col_i[a]->name, col_j[b]->name);
                    addEdge(g, createEdge(name, col_i[a], col_j[b]));
                }
            }
        }

        free(col_i);
        free(col_j);
    }

    // ----------------------------------------------------------------------
    // Relier les X_{i-1} et X_{i+1} s'ils n'ont pas de voisin 's' commun
    // ----------------------------------------------------------------------
    for (int i = 2; i <= 2 * path; i += 2) {
        int leftIdx  = i - 1;   // colonne impaire gauche
        int rightIdx = i + 1;   // colonne impaire droite
        if (leftIdx < 1 || rightIdx > 2 * path + 1) continue;

        // ----- Collecter TOUS les sommets de la colonne gauche et de la colonne droite -----
        Vertex** leftAll  = NULL; int cntL = 0;
        Vertex** rightAll = NULL; int cntR = 0;

        for (int v = 0; v < g->nbv; v++) {
            Vertex* vert = g->vertices[v];
            int base = getBaseIndex(vert->name);
            if (base == leftIdx) {
                leftAll = realloc(leftAll, (cntL + 1) * sizeof(Vertex*));
                leftAll[cntL++] = vert;
            } else if (base == rightIdx) {
                rightAll = realloc(rightAll, (cntR + 1) * sizeof(Vertex*));
                rightAll[cntR++] = vert;
            }
        }

        // 1) Connexion M_{i-1} ↔ M_{i+1} 
        for (int a = 0; a < cntL; a++) {
            if (leftAll[a]->name[0] != 'm') continue;
            for (int b = 0; b < cntR; b++) {
                if (rightAll[b]->name[0] != 'm') continue;
                if (!edgeExists(g, leftAll[a], rightAll[b]) &&
                    !shareCommonS(g, leftAll[a], rightAll[b]))
                {
                    snprintf(name, BUFFER_SIZE, "e_%s_%s",
                            leftAll[a]->name, rightAll[b]->name);
                    addEdge(g, createEdge(name, leftAll[a], rightAll[b]));
                }
            }
        }

        // 2) L_{i-1} → tous les sommets de droite (i+1)
        for (int a = 0; a < cntL; a++) {
            if (leftAll[a]->name[0] != 'l') continue;
            for (int b = 0; b < cntR; b++) {
                if (!edgeExists(g, leftAll[a], rightAll[b])) {
                    snprintf(name, BUFFER_SIZE, "e_%s_%s",
                            leftAll[a]->name, rightAll[b]->name);
                    addEdge(g, createEdge(name, leftAll[a], rightAll[b]));
                }
            }
        }

        // 3) R_{i+1} → tous les sommets de gauche (i-1)
        for (int b = 0; b < cntR; b++) {
            if (rightAll[b]->name[0] != 'r') continue;
            for (int a = 0; a < cntL; a++) {
                if (!edgeExists(g, rightAll[b], leftAll[a])) {
                    snprintf(name, BUFFER_SIZE, "e_%s_%s",
                            rightAll[b]->name, leftAll[a]->name);
                    addEdge(g, createEdge(name, rightAll[b], leftAll[a]));
                }
            }
        }

        free(leftAll);
        free(rightAll);
    }

    // ======================================================================
    // 3. Relier les colonnes i et j telles que :
    //    j >= i+3  et  (j - i) % 3 == 2
    // ======================================================================
    for (int i = 1; i <= 2 * path; i += 1) {
        for (int j = i + 3; j <= 2 * path+1; j += 1) {   // j pair, au moins i+3
            if ((j - i) % 3 != 2) continue;            // écart ≡ 2 mod 3

            // Collecter tous les sommets de type s d'indice de base i et j
            Vertex** col_i = NULL; int cnt_i = 0;
            Vertex** col_j = NULL; int cnt_j = 0;

            for (int v = 0; v < g->nbv; v++) {
                Vertex* vert = g->vertices[v];
                
                int base = getBaseIndex(vert->name);     // extrait l'indice principal
                if (base == i) {
                    col_i = realloc(col_i, (cnt_i + 1) * sizeof(Vertex*));
                    col_i[cnt_i++] = vert;
                } else if (base == j) {
                    col_j = realloc(col_j, (cnt_j + 1) * sizeof(Vertex*));
                    col_j[cnt_j++] = vert;
                }
            }

            // Créer toutes les arêtes manquantes entre les deux colonnes
            for (int a = 0; a < cnt_i; a++) {
                for (int b = 0; b < cnt_j; b++) {
                    if (!edgeExists(g, col_i[a], col_j[b])) {
                        snprintf(name, BUFFER_SIZE, "e_%s_%s",
                                col_i[a]->name, col_j[b]->name);
                        addEdge(g, createEdge(name, col_i[a], col_j[b]));
                    }
                }
            }

            free(col_i);
            free(col_j);
        }
    }

    // ----------------------------------------------------------------------
    // Suppression aléatoire d'arêtes entre les sommets de type sX_1_Y
    // ----------------------------------------------------------------------
    const double removeRate = 0.1   ;   // 40% de suppression

    if (removeRate > 0.0) {
        // Première passe : identifier les arêtes à supprimer
        // (on ne peut pas modifier le tableau edges pendant qu'on le parcourt)
        bool* toRemove = calloc(g->nbe, sizeof(bool));
        for (int i = 0; i < g->nbe; i++) {
            Edge* e = g->edges[i];
            Vertex* v1 = e->endpoints[0];
            Vertex* v2 = e->endpoints[1];
            // Vérifier que les deux sommets sont bien des "sX_1_Y"
            if (v1->name[0] != 's' || v2->name[0] != 's') continue;
            int base1, subsub1;
            int base2, subsub2;
            // Format : s%d_1_%d
            if (sscanf(v1->name, "s%d_1_%d", &base1, &subsub1) != 2) continue;
            if (sscanf(v2->name, "s%d_1_%d", &base2, &subsub2) != 2) continue;
            // Arête entre deux négatifs : tirage aléatoire
            if ((rand() % 100) < (int)(removeRate * 100)) {
                toRemove[i] = true;
            }
        }

        // Deuxième passe : supprimer les arêtes marquées (en partant de la fin)
        for (int i = g->nbe - 1; i >= 0; i--) {
            if (toRemove[i]) {
                deleteEdge(g, *g->edges[i]);   // utilise votre fonction deleteEdge existante
            }
        }
        free(toRemove);
    }









    // Libération des tableaux temporaires
    for (int i = 0; i < path; i++) {
        free(L_pairs[i]);
        free(R_pairs[i]);
    }
    free(L_pairs);
    free(R_pairs);
    free(nbLR_list);
    free(pathVertices);
    free(edgeCounters);
    free(multiplied);


    int nbTrianglesFinaux =countTriangle( g);
    //printf("NOMBRE DE TRIANGLES INITIAUX:%d. NOMBRE DE TRIANGLES FINAUX: %d\n",nbTriangles,nbTrianglesFinaux);

    return g;
}


/**
 * Parse un fichier DOT (format généré par generateDotFile) et reconstruit
 * le graphe complet du PathOfTriangle correspondant.
 *
 * @param dotFilename Nom du fichier .dot à lire
 * @return Un graphe alloué dynamiquement, ou NULL en cas d'erreur.
 */
Graph* parsePathOfTriangle(const char* dotFilename) {
    FILE* f = fopen(dotFilename, "r");
    if (!f) {
        fprintf(stderr, "Impossible d'ouvrir %s\n", dotFilename);
        return NULL;
    }

    char line[512];
    int nbVertices = 0, maxBase = 0;

    // Première passe : compter les sommets et déterminer maxBase
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "[shape=circle]")) {
            nbVertices++;
            char name[128];
            if (sscanf(line, "\t%127s [", name) == 1) {
                int base = getBaseIndex(name);
                if (base > maxBase) maxBase = base;
            }
        }
    }
    if (nbVertices == 0 || maxBase < 2) { fclose(f); return NULL; }

    Graph* g = createGraph("ParsedPoT", nbVertices);
    rewind(f);

    // Deuxième passe : ajouter les sommets
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "[shape=circle]")) {
            char name[128];
            if (sscanf(line, "\t%127s [", name) == 1) {
                addVertex(g, createVertex(name));
            }
        }
    }

    // Troisième passe : lire les arêtes existantes dans le DOT
    rewind(f);
    while (fgets(line, sizeof(line), f)) {
        // Format attendu : "\t%s -- %s [éventuellement attributs];\n"
        char v1_name[128], v2_name[128];
        if (sscanf(line, "\t%127s -- %127s", v1_name, v2_name) == 2) {
            // Enlever le ';' final éventuel collé à v2_name
            char* semi = strchr(v2_name, ';');
            if (semi) *semi = '\0';
            // Ignorer les arêtes invisibles (style=invis)
            if (strstr(line, "style=invis")) continue;

            Vertex* v1 = getVertex(g, v1_name);
            Vertex* v2 = getVertex(g, v2_name);
            if (v1 && v2 && !edgeExists(g, v1, v2)) {
                char edgeName[256];
                snprintf(edgeName, sizeof(edgeName), "e_%s_%s", v1_name, v2_name);
                addEdge(g, createEdge(edgeName, v1, v2));
            }
        }
    }
    fclose(f);

    int path = maxBase / 2;

    // Appliquer maintenant les règles pour les arêtes non présentes dans le DOT
    // (elles n'ont pas été affichées car |base1 - base2| != 1)

    // 1) Relier les M_{i-1} ↔ M_{i+1} (s'ils n'ont pas de s commun) et les L/R
    for (int i = 2; i <= 2 * path; i += 2) {
        int leftIdx  = i - 1;
        int rightIdx = i + 1;
        if (leftIdx < 1 || rightIdx > 2 * path + 1) continue;

        Vertex** leftAll = NULL;  int cntL = 0;
        Vertex** rightAll = NULL; int cntR = 0;
        for (int v = 0; v < g->nbv; v++) {
            Vertex* vert = g->vertices[v];
            int base = getBaseIndex(vert->name);
            if (base == leftIdx) {
                leftAll = realloc(leftAll, (cntL+1)*sizeof(Vertex*));
                leftAll[cntL++] = vert;
            } else if (base == rightIdx) {
                rightAll = realloc(rightAll, (cntR+1)*sizeof(Vertex*));
                rightAll[cntR++] = vert;
            }
        }

        // a) M ↔ M (sans voisin s commun)
        for (int a = 0; a < cntL; a++) {
            if (leftAll[a]->name[0] != 'm') continue;
            for (int b = 0; b < cntR; b++) {
                if (rightAll[b]->name[0] != 'm') continue;
                if (!edgeExists(g, leftAll[a], rightAll[b]) &&
                    !shareCommonS(g, leftAll[a], rightAll[b])) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "e_%s_%s",
                             leftAll[a]->name, rightAll[b]->name);
                    addEdge(g, createEdge(buf, leftAll[a], rightAll[b]));
                }
            }
        }

        // b) L_{i-1} → tous les sommets de droite
        for (int a = 0; a < cntL; a++) {
            if (leftAll[a]->name[0] != 'l') continue;
            for (int b = 0; b < cntR; b++) {
                if (!edgeExists(g, leftAll[a], rightAll[b])) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "e_%s_%s",
                             leftAll[a]->name, rightAll[b]->name);
                    addEdge(g, createEdge(buf, leftAll[a], rightAll[b]));
                }
            }
        }

        // c) R_{i+1} → tous les sommets de gauche
        for (int b = 0; b < cntR; b++) {
            if (rightAll[b]->name[0] != 'r') continue;
            for (int a = 0; a < cntL; a++) {
                if (!edgeExists(g, rightAll[b], leftAll[a])) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "e_%s_%s",
                             rightAll[b]->name, leftAll[a]->name);
                    addEdge(g, createEdge(buf, rightAll[b], leftAll[a]));
                }
            }
        }

        free(leftAll); free(rightAll);
    }

    // 2) Arêtes entre colonnes i et j (j ≥ i+3, (j-i)%3 == 2)
    for (int i = 1; i <= 2 * path + 1; i++) {
        for (int j = i + 3; j <= 2 * path + 1; j++) {
            if ((j - i) % 3 != 2) continue;
            Vertex** col_i = NULL; int cnt_i = 0;
            Vertex** col_j = NULL; int cnt_j = 0;
            for (int v = 0; v < g->nbv; v++) {
                Vertex* vert = g->vertices[v];
                int base = getBaseIndex(vert->name);
                if (base == i) {
                    col_i = realloc(col_i, (cnt_i+1)*sizeof(Vertex*));
                    col_i[cnt_i++] = vert;
                } else if (base == j) {
                    col_j = realloc(col_j, (cnt_j+1)*sizeof(Vertex*));
                    col_j[cnt_j++] = vert;
                }
            }
            for (int a = 0; a < cnt_i; a++) {
                for (int b = 0; b < cnt_j; b++) {
                    if (!edgeExists(g, col_i[a], col_j[b])) {
                        char buf[256];
                        snprintf(buf, sizeof(buf), "e_%s_%s",
                                 col_i[a]->name, col_j[b]->name);
                        addEdge(g, createEdge(buf, col_i[a], col_j[b]));
                    }
                }
            }
            free(col_i); free(col_j);
        }
    }

    return g;
}