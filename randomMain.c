
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "Graph.h"


int main(){
    srand(time(NULL));

    Graph* g=PathOfTriangle(20,1,5,5);
    generateDotFile(g,"graphes/PoT1.dot");
}