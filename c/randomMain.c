
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

    int path=14;
    Cover c;
    int j;
    
    int count[6]={0,0,0,0,0,0};
    for (int i=0;i<100000;i++){
        path=rand()%10+10;
        Graph* g=PathOfTriangle(path,1,15,10);
        int* check = checkPathOfTriangleProperties(g);
        printList(check,7);
        free(check);
        


        c=couvetureLR(g,path);

        
        j=countVertices(c.S);
        if (j>2){
            generateDotFileWithTriplets(g,"graphes/CE22SI.dot",c.T);
            printf("LR,path=%d, %d sommets isolés\n",path,j);
            exit(1);
        }
    
        c=couvetureM(g,path);

        
        j=countVertices(c.S);
        if (j>2){
            generateDotFileWithTriplets(g,"graphes/CE22SI.dot",c.T);
            printf("M,path=%d, %d sommets isolés\n",path,j);
            exit(1);
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