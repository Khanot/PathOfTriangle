
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
    srand(time(NULL));

    int path=5;
    
    int count[6]={0,0,0,0,0,0};
    for (int i=0;i<1;i++){
        Graph* g=PathOfTriangle(path,1,3,3);
        int* check = checkPathOfTriangleProperties(g);
        printList(check,7);
        free(check);
        generateDotFile(g,"graphes/PoT1.dot");
    
        Cover c=couveture(g,path);

        int j=countVertices(c.S);
        if (j<5){
            count[j]++;
        }
        else{
            count[5]++;
        }
    }
    printList(count,6);

}