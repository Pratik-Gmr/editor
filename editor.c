// library inclusion
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

// Function to mimic getch() kept on top cause I have habit of taking getch as library function
char getch() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt); // Get current terminal attributes
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new settings
    ch = getchar(); // Read a single character
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore old settings
    return ch;
}


// Define macros
#define True 1
#define False 0

// defining new data types, global variables, new structures
typedef char* string;
typedef struct node{
    string line;
    struct node* prev;
    struct node* next;
} node;


// Function declaration (sequentially)
int error(string message,int exit_status);
node* create_node();
FILE* create_empty_file(string name);
int load_to_buffer(FILE* f, node* head);
int display_buffer(node* head);
int update_buffer(node* head,int line_no,string line);
node* go_to(node* head,int line_no);
int update_to_file(FILE* f, node* head);
int free_buffer(node* head);

// main
int main(int argc, string argv[]){
    int return_value, list_length;
    string exit_message;
    node* buffer;
    // taking file_name
    if (argc != 2){
        string exit_message = "Error: Wrong command\n Command should be in format: './editor test.txt'\n";
        return error(exit_message,1);
    }
    string file_name = argv[1];
    FILE* file;
    // opening file for reading
    file = fopen(file_name,"r");
    if (file == NULL){
        // load empty buffer
        free(file);
        file = create_empty_file(file_name);
        printf("New file created.\n");
    }
    // laod file content to buffer
    buffer = create_node();
    if (buffer == NULL){
        return_value = 1;
    }
    else{
        return_value = load_to_buffer(file, buffer);
    }
    if (return_value != 0){
        exit_message = "Error: Problem occured during buffer-loading.\nPlease try again later.\n";
        return error(exit_message,return_value);
    }
    fclose(file);
    string new_line = (string)malloc(4096);
    while(True){
        unsigned int line_no;
        list_length = display_buffer(buffer);
        printf("Which line you wish to change?\n");
        scanf(" %u", &line_no);
        // Clear the input buffer after using scanf
        while (getchar() != '\n'); 
        if (line_no == 0){
            break;
        }
        else if(line_no > list_length +1){
            printf("line %d doesnt exist\n",line_no);
            getch();
            continue;
        }
        printf("What change?\n");
        fgets(new_line, 1024, stdin);//because scanf just breaks and gets is unreliable
        new_line[strcspn(new_line, "\n")] = '\0';//removing trailing new line
        return_value = update_buffer(buffer,line_no,new_line);
        if(return_value != 0){
            exit_message = "Error:\n";
            return error(exit_message,return_value);
        }
    }
    free(new_line);
    // write to file
    file = fopen(file_name,"w");
    if (file == NULL){
        return_value = 404;
    }
    else{
        // write updated buffer to file
        return_value = update_to_file(file, buffer);
    }
    if (return_value != 0){
        exit_message = "Error: Problem occured during saving to file: %s\nFile might be unchanged\n";
        return error(exit_message, return_value);
    }
    fclose(file);
    return_value = free_buffer(buffer);
    if (return_value != 0){
        exit_message = "Error: Problem occured during buffer-freeing.\nThere might be memory leaks.\n";
        return error(exit_message,return_value);
    }
    printf("file: %s is updated.\n", file_name);
    return 0;
}

// functions
int error(string message, int exit_status){
    printf("%s", message);
    return exit_status;
}

node* create_node(){
    node* Node = (node*)malloc(sizeof(node));
    Node->line = NULL;
    Node->prev = NULL;
    Node->next = NULL;
    return Node;
}

FILE* create_empty_file(string name){
    FILE* file = fopen(name, "w");
    fprintf(file," \n");
    fclose(file);
    file = fopen(name, "r");
    return file;
}

int load_to_buffer(FILE* f,node* head){
    int c;
    node* current = head;
    int line_length = 0;
    size_t line_size = 1;
    while((c = fgetc(f)) != EOF){
        // for new line
        if (current->line == NULL){
            current->line = (string)malloc(line_size);
            if(current->line == NULL){
                return 1;
            }
            current->line[0] = '\0';
        }
        // for older line increasing line buffer size if content doesnt fit-in
        else if(line_length +1 >= line_size){
            line_size += 1;            
            current->line = (string)realloc(current->line, line_size);
            if(current->line == NULL){
                return 1;//error
            }
        }
        if(c == '\n'){
            // create new node for new line
            node* new_node = create_node();
            if(new_node == NULL){
                return 1;
            }
            current->next = new_node;
            new_node->prev = current;
            current = current->next;
            line_length = 0;
            line_size = 1;
        }
        else{
            // for old line append every character of that line
            current->line[line_length] =(char) c;
            line_length++;
            current->line[line_length] = '\0'; //null termination for no error during string handeling
        }
    }
    //Removing last empty node if created
    if (current->line == NULL)
        free(current->line);
        current = current->prev;
        free(current->next);
        current->next = NULL;
    return 0;//no error
}   

int display_buffer(node* head){
    system("clear");
    node* current = head;
    int length;
    for(length = 0;current != NULL;length++){
        printf("%d\t%s \n",length+1, current->line);
        current = current->next;
    }
    return length;
}

int update_buffer(node* head,int line_no,string line){
    node* current = go_to(head, line_no);
    printf("Puginggg1\n");
    current->line = (string)realloc(current->line,strlen(line)*sizeof(char)+1);
    if(current->line == NULL){
        return 1;
    }
    printf("Puginggg1\n");
    strcpy(current->line,line);
    printf("Puginggg1\n");
    return 0;
}

node* go_to(node* head, int line_no){
    node* current = head;
    for(int i = 1; i < line_no; i++){
        if(current->next == NULL && i == line_no-1){
            current->next = create_node();
        }
        current = current->next;
    }
    return current;
}

int update_to_file(FILE* f, node* head){
    node* current = head;
    if(f == NULL || current == NULL){
        return 1;
    }
    while(current != NULL){
        fprintf(f,"%s\n",current->line);
        current = current->next;
    }
    return 0;
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
