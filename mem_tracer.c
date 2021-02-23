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
    int index;
    char* command;
    struct node* next;
}node;
node* malloc_node(int size, char* command, int index);
node* realloc_node(node* pointer, int size, char* command, int index);
void free_node(node* pointer);
node* search_command(char* command, node* head);

node * create_node(int index, char* command){
    node* new = malloc_node(sizeof(node), command, index);
    new-> index = index;
    new-> command = command;
    new-> next = NULL;
    return new;
}
void print_node(node* head){
    node* temp = head;
    while (temp != NULL){
        fprintf(stdout,"--->index %d, line %s", temp->index, temp->command);
        temp = temp->next;
    }
}

node* malloc_node(int size, char* command, int index){
    node *pointer;
    pointer = (node*)malloc(size);
    fprintf(stdout,"Command %s at index %d allocated new memory segment at address %p to size %d\n", command, index, pointer, size);
    return pointer;
}
node* realloc_node(node* pointer, int size, char* command, int index) {
    pointer = realloc(pointer, size);
    fprintf(stdout,"Command %s at index %d reallocated new memory segment at address %p to a new size %d\n", command, index, pointer, size);
    return pointer;
}
char** realloc_array(char** pointer, int size){
    pointer = realloc(pointer, size);
    fprintf(stdout,"Array reallocated new memory segment at address %p to a new size %d\n", pointer, size);
    return pointer;
}
void free_node(node* pointer){
    fprintf(stdout,"Command %s at index %d deallocated the memory segment at at address %p\n", pointer->command, pointer->index, pointer);
    free(pointer);
}
void free_list(node* head){
    node* temp = head;
    while (temp != NULL){
        free_node(temp);
        temp = temp->next;
    }
}
void free_array(char** pointer){
    fprintf(stdout,"Array deallocated the memory segment at at address %p\n", pointer);
    free(pointer);
}
void add_node(node*current, node* next){
    current->next = next;
}
int main(int argc, char *argv[]) {
    FILE * fp;
    int initial_size = 4;
    char * line = NULL;
    char** array = malloc(sizeof(char*)*initial_size);
    size_t len = 0;
    ssize_t read;
    int index = 0;
    if(argv[1]==NULL){
        perror("File do not exist");
    }
    fp = fopen(argv[1], "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    node* head = NULL;
    node* current= NULL;
    int fd = open("memtrace.out", O_APPEND | O_CREAT | O_RDWR, 0777);
    dup2(fd,1);
    while ((read = getline(&line, &len, fp)) != -1) {
        index++;
        if(index> initial_size)
            array = realloc_array(array,sizeof(char*)*index);
        char *command = malloc(read);
        memcpy(command, line, read);
        array[index-1] = command;
    }
    for(int i = 0; i<index; i++){
        node* temp = create_node(i+1, array[i]);
        if(head == NULL)
            head = current = temp;
        else {
            add_node(current, temp);
            current = temp;
        }
    }
    free_list(head);
    free_array(array);
    fclose(fp);
}
