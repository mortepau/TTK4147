#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"

int main(int argc, char* argv[]){

    if(strcmp(argv[1], "A") == 0){
        long xy_size = 1000*1000*1000;
        long x_dim = 10000;
        long y_dim = xy_size/x_dim;

        long** matrix = malloc(y_dim*sizeof(long*));

        for(long y = 0; y < y_dim; y++){
            matrix[y] = malloc(x_dim*sizeof(long));
            memset(matrix[y], 0, x_dim*sizeof(long));
        }

        printf("Allocation complete (press any key to continue...)\n");
        getchar();
    }
    if(strcmp(argv[1], "B") == 0){
        Array a = array_new(2);
        long times = 1000*1000;

        for(long i = 0; i < times; i++){
            array_insertBack(&a, i);
        }
        //array_print(a);
    }
}