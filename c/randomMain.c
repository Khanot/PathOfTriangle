
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

    int path=30;
    Cover c;
    int j;
    
    int count[6]={0,0,0,0,0,0};
    for (int i=0;i<100000;i++){
        path=rand()%10+10;
        Graph* g=PathOfTriangle(path,1,10,10);
        int* check = checkPathOfTriangleProperties(g);
        //printList(check,7);
        free(check);
        

    
        c=couvetureManuelle(g,path);

        
        int s=countIsolatedVertices(&c);
        if (s>4){
            printIsolatedVertices(&c);
            generateDotFileSimplified(g,"graphes/aled.dot");
            exit(0);
        }
        
        /*if (j<5){
            count[j]++;
        }
        else{
            count[5]++;
        }*/
    }
    printList(count,6);

}