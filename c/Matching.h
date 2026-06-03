#ifndef MATCHING_H
#define MATCHING_H

#include "Graph.h"

/**
 * Calcule un couplage maximum exact par l’algorithme d’Edmonds (Blossom).
 * Complexité O(V^3) en pratique, bien inférieure à O(V^2 * E).
 *
 * @param g      Graphe non orienté. Les sommets doivent avoir un champ `id`
 *               valide (leur indice dans g->vertices).
 * @param match  Tableau pré‑alloué de taille g->nbv.
 *               En sortie, match[u] = l’indice du partenaire, ou -1 si u est libre.
 * @return       Nombre d’arêtes du couplage maximum.
 */
int maximumMatching(Graph* g, int* match);


int maximumMatchingBad(Graph* g, int* match);
#endif