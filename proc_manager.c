//---------------------------
//put commands in a data structure (e.g. linkedlist), which is used
//for recording the start times and indices of the commands.
//see the read_parse_file.c I provided for parsing each command
//save the cmd in a node of the linkedlist

//First command node (head of list)
#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <time.h>

#define TABLE_SIZE 10
typedef struct node{
    int pid;
    int index;
    int ctr;
    struct timespec start;
    char command[20][20];
    struct node* next;
}node;
node * create_node(int index, char command[20][20], int ctr){
    node* new = malloc(sizeof(node));
    new-> index = index;
    new-> next = NULL;
    new-> ctr = ctr;
    for(int i = 0; i< 20; i++){
        for(int j =0; j<20; j++) {
            new->command[i][j] = command[i][j];
        }
    }
    return new;
}
void change_pid(node* node, int pid){
    node->pid = pid;
}
void print_table(node* head){
    while(head!=NULL){
        printf("index: %d\n", head->index);
        printf("pid: %d\n", head->pid);
        for(int i = 0; i< 20; i++) {
            printf("%s ", head-> command[i]);
        }
        head = head->next;
    }
}
node* find_node(int pid, node* head) {
    node *temp = head;
    while (temp != NULL) {
        if (pid == temp->pid)
            return temp;
        else {
            temp = temp->next;
        }
    }
    return NULL;
}

void execute(node *head) {
    node *current;
    pid_t pid;
    int status;
    current = head;
    while (current != NULL) {
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
        current->start = start;
        pid = fork();
        if (pid < 0) {
            perror("fork error");
        } else if (pid == 0) {        /* child */
            char name[64];
            change_pid(current, getpid());
            sprintf(name, "%d.out", current->pid);
            int file = open(name, O_APPEND | O_CREAT | O_RDWR, 0777);
            dup2(file, 1);
            fprintf(stdout, "Starting command %d: child %d pid of parent %d\n", current->index, current->pid,
                    getppid());
            fflush(stdout);
            close(file);
            char *args[20];
            for (int i = 0; i < current->ctr; i++)
                args[i] = current->command[i];
            char *a = args[current->ctr - 1];
            a[strlen(a) - 1] = '\0';
            args[current->ctr] = NULL;
            if (execvp(args[0], args) < 0) {
                perror("exec error");
            }
            exit(1);

        } else if (pid > 0) {
            change_pid(current, pid);
            current = current->next;
        }
    }
    while ((pid = wait(&status)) >= 0) {
        if (pid > 0) {
            double elapsedtime;
            struct timespec finish;
            char name[64];

            clock_gettime(CLOCK_MONOTONIC, &finish);
            node *found = find_node(pid, head);
            if (found == NULL) {
                perror("no node with this pid exist");
            }
            sprintf(name, "%d.out", found->pid);
            int file = open(name, O_APPEND | O_CREAT | O_RDWR, 0777);
            dup2(file, 1);
            fprintf(stdout, "Finished child %d pid of parent %d\n", found->pid, getpid());
            fflush(stdout);
            close(file);
            char name2[64];
            sprintf(name2, "%d.err", found->pid);
            int fderr = open(name2, O_APPEND | O_CREAT | O_RDWR, 0777);
            dup2(fderr, 2);
            if (WIFEXITED(status)) {
                fprintf(stderr, "Exited with exit code: %d\n", WEXITSTATUS(status));
                close(fderr);
            } else if (WIFSIGNALED(status)) {
                fprintf(stderr, "Killed with signal number: %d\n", WTERMSIG(status));
                close(fderr);
            }
            elapsedtime = (finish.tv_sec - found->start.tv_sec);
            fprintf(stdout, "Finished at %ld, runtime duration %f\n", finish.tv_sec, elapsedtime);
            fflush(stdout);
            if (elapsedtime <= 2) {
                fprintf(stderr, "spawning too fast\n");
            }
        }
    }
}

int main(int argc, char *argv[]) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char newString[20][20];
    int i,j,ctr;
    int index = 0;
    if(argv[1]==NULL){
        perror("File do not exist");
    }
    fp = fopen(argv[1], "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    node* head = NULL;
    node* current= NULL;
    while ((read = getline(&line, &len, fp)) != -1) {
        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 20; j++) {
                newString[i][j] = 0;
            }
        }
        j = 0;
        ctr = 0;
        for (i = 0; i <= (strlen(line)); i++) {
            // if space or NULL found, assign NULL into newString[ctr]
            if (line[i] == ' ' || line[i] == '\0') {
                newString[ctr][j] = '\0';
                ctr++;  //for next word
                j = 0;    //for next word, init index to 0
            } else {
                newString[ctr][j] = line[i];
                j++;
            }
        }
        index++;
        node* temp = create_node(index, newString, ctr);
        if(head == NULL)
            head = current = temp;
        else
            current = current->next = temp;
    }
    execute(head);
    fclose(fp);
}
