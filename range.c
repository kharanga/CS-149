#include <stdio.h>
#include <float.h>
#include <stdlib.h>
int main(int argc, char *argv[]){
    FILE * fp;
    double min = DBL_MAX;
    double max = DBL_MIN;
    double value;
    if(argc < 2){
        exit(0);
    }
    fp = fopen(argv[1],"r");
    if(fp == NULL){
        printf("avg: cannot open the input file\n");
        exit(1);
    }
    while(fscanf(fp, "%lf", &value)!=EOF){
        if(value>max){
            max = value;
        }
        if(value<min){
            min = value;
        }
    }
    printf("min id %lf, max is %lf.", min, max);
    fclose(fp);
    return 0;
}