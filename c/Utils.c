#include "Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


void printList(int* l, int s){
    printf("[");
    for (int i=0;i<s;i++){
        printf("%d,",l[i]);
    }
    printf("]");
}