
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "Graph.h"
#include "Algo.h"
#include "Utils.h"
#include "PathOfTriangleCHeck.h"

int main(){
    Graph* g =parsePathOfTriangle("graphes/CE13SI.dot");
    int* check = checkPathOfTriangleProperties(g);
    printList(check,7);
    free(check);
    Cover c;
    int j;
    int path=13;
    

    CoverListNode* solutions = couvertureAll(g, path);
    //printCoverListSizes(solutions);
    
    Cover* c1= getNthCover(solutions, 44) ;
    Cover* c2= getNthCover(solutions, 79) ;
    generateDotFileWithTriplets(g,"graphes/potbad.dot",c1->T);
    generateDotFileWithTriplets(g,"graphes/potgood.dot",c2->T);
    for (int i=44;i<81;i+=2){
        c1= getNthCover(solutions, i) ;
        printIsolatedVertices(c1);
    }
    
    
    freeCoverList(solutions);
      
    
    
    
}