// library inclusion
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define macros
#define True 1
#define False 0

// defining new data types, global variables, new structures
typedef char* string;
typedef char bool;
typedef struct node{
    string line;
    struct node* next;
} node;


// Function declaration (sequentially)
int error(string message,int exit_status);
node* create_node();
int load_to_buffer(FILE* f, node* head);
void display_buffer(node* head);
int free_buffer(node* head);

// main
int main(int argc, string argv[]){
    // taking file_name
    if (argc != 2){
        string exit_message = "Error: Wrong command\n Command should be in format: './editor test.txt'\n";
        return error(exit_message,1);
    }
    string file_name = argv[1];
    FILE* file;
    // opening file for
    file = fopen(file_name,"r");
    if (file == NULL){
        // load empty buffer
    }
    else{
        // laod file content to buffer
        node* buffer = create_node()
        int return_value = load_to_buffer(file, buffer);
        if (return_value != 0){
            string exit_message = "Error: Problem occured during buffer-loading.\nPlease try again later.\n";
            return error(exit_message,return_value);
        }
        fclose(file);
    }
    
    display_buffer(buffer);

    // write to file
    file = fopen(file_name,"w");
    // write updated buffer to file
    printf("Writing file: %s\n", file_name);
    fclose(file);
    int return_value = free_buffer(buffer);
    if (return_value != 0){
        string exit_message = "Error: Problem occured during buffer-freeing.\nThere might be memory leaks.\n";
        return error(exit_message,return_value);
    }
    return 0;
}

// functions
int error(string message, int exit_status){
    printf("%s", message);
    return exit_status;
}

node* create_node(){
    node* Node = (node*)malloc(sizeof(node));
    Node->next = NULL;
    return Node;
}

//TODO the following function s terribly implemented and needs fixes, am keaving this for when I am not sleepy and half-brain-dead
int load_to_buffer(FILE* f,node* head){
    char c = fgetc(f);
    node* current = head;
    if(c == EOF){
        return 0;
    }
    if(c == '\n'){
        strcat(current->line,'\0');
        current->next = create_node();
        head = current->next;
    }else{
        strcat(current->line,&c);
    }
    return load_to_buffer(current,head);
}

void display_buffer(node* head){
    node* current = head;
    for(int i = 1;current != NULL;i++){
        printf("%d\t%s \n",i, current->line);
        current = current->next;
    }
}

int free_buffer(node* head) {
    node* current = head;
    while (current != NULL) {
        node* temp = current->next;
        if (current->line != NULL) {
            free(current->line);
        }
        free(current);
        current = temp;
    }
    return 0;
}