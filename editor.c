// library inclusion
#include <stdio.h>
#include <string.h>

// Define macros
#define True 1
#define False 0

// defining new data types, global variables, new structures
typedef char* string;

// Function declaration (sequentially)
int error(string message,int exit_status);

// main
int main(int argc, string argv[]){
    // taking file_name
    if (argc != 2){
        string exit_message = "Error: Wrong command\n Command should be in format: './editor test.txt'\n";
        return error(exit_message,1);
    }
    string file_name = argv[1];

    // opening file for reading
    FILE* file = fopen(file_name,"r");
    if (file == NULL){
        // load empty buffer
    }
    else{
        // laod file content to buffer
        fclose(file);
    }
    
    return 0;
}

// functions
int error(string message, int exit_status){
    printf("%s", message);
    return exit_status;
}