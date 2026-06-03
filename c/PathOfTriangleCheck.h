#ifndef PATHOFTRIANGLECHECK_H
#define PATHOFTRIANGLECHECK_H

#include "Graph.h"

/**
 * Vérifie les sept propriétés caractéristiques d’un Path of Triangle.
 * @param g Graphe à tester
 * @return Un tableau de 7 entiers alloué dynamiquement (indices 0 à 6).
 *         result[i] = 1 si la propriété (Pi+1) est vérifiée, 0 sinon.
 *         Retourne NULL en cas d’erreur d’allocation.
 *         L’appelant est responsable de libérer le tableau avec free().
 */
int* checkPathOfTriangleProperties(Graph* g);

#endif