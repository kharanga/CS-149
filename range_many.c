#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    FILE * fp;
    int num_files = argc-1;
    double min = DBL_MAX;
    double max = DBL_MIN;
    double x, y, value;
    int pipe_a[2], pipe_b[2];
    pid_t p;

    //create pipe a and b and check for error
    if(pipe(pipe_a)==-1){
       fprintf(stderr, "Pipe Failed");
        return 1;
    }
    if(pipe(pipe_b)==-1){
        fprintf(stderr, "Pipe Failed");
       return 1;
    }

    if(argc < 2){
        exit(0);
    }

    //loop through the parameter and read all files
    for(int i  = 1; i <= num_files; i++){
        double lmax = DBL_MIN;
        double lmin = DBL_MAX;
        p = fork();
        if(p<0) {
            fprintf(stderr, "Fork Failed");
        }
        else if(p==0) {
            fp = fopen(argv[i], "r");
            if (fp == NULL) {
                printf("avg: cannot open the input file\n");
                exit(1);
            }
            //loop and read all the inputs in the file
            while (fscanf(fp, "%lf", &value) != EOF) {
                if (value > lmax) {
                    lmax = value;
                }
                if (value < lmin) {
                    lmin = value;
                }
            }
            fclose(fp);
            //write local min in pipe a and local max in pipe b
            write(pipe_a[1], &lmin, sizeof(lmin));
            write(pipe_b[1], &lmax, sizeof(lmax));
            exit(0);
        }
        else{
            //wait for the child to finish passing in the local max and min
            wait(NULL);

            //read its child local max and min and compare it to the global max and min
            read(pipe_a[0], &x, sizeof(x));
            read(pipe_b[0], &y, sizeof(y));
            if(x<min){
                min = x;
            }
            if(y>max){
                max = y;
            }
        }
    }
    // close pipe a and b
    close(pipe_a[0]);
    close(pipe_a[1]);
    close(pipe_b[0]);
    close(pipe_b[1]);
    printf("min: %fl\nmax: %fl\n", min, max);
    return 0;
}