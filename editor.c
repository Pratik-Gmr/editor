// library inclusion
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
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
typedef struct cursor{
    int row;
    int column;
} cursor;


// Function declaration (sequentially)
int error(string message,int exit_status);
node* create_node();
FILE* create_empty_file(string name);
int load_to_buffer(FILE* f, node* head);
int editor(node* buffer);
int delete(node* current);
int backspace(node* current);
void signal_handler(int signum);
int display_buffer(node* head);
int update_buffer(node* head,char new_char);
node* go_to(node* head,int line_no);
int update_to_file(FILE* f, node* head);
int free_buffer(node* head);

// global variables
static int Exit = False;
// cursor defination
static cursor CURSOR = {0,0};
void print_cursor(char c){
    printf("\x1b[7m%c\x1b[0m",c);
}

// main
int main(int argc, string argv[]){
    system("clear");
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

    return_value = editor(buffer);
    if(return_value != 0){
        exit_message = "Error: The progran has been aborted\n";
        return error(exit_message,return_value);
    }

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
        exit_message = "Error: Problem occured during saving to file\nFile might be unchanged\n";
        return error(exit_message, return_value);
    }
    fclose(file);

    return_value = free_buffer(buffer);
    if (return_value != 0){
        exit_message = "Error: Problem occured during buffer-freeing.\nThere might be memory leaks.\n";
        return error(exit_message,return_value);
    }signal
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
    if(Node != NULL){
        Node->line = NULL;
        Node->prev = NULL;
        Node->next = NULL;
    }
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
        if (current->line == NULL){ //initialize the new line if empty
            current->line = (string)malloc(line_size);
            if(current->line == NULL){
                return 1;
            }
            current->line[0] = '\0';
        }
        // for older line increasing line buffer size if content doesnt fit-in
        else if(line_length +1 >= line_size){
            line_size += 2;            
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
            //initialize values for new line
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
    if (current->line == NULL){
        free(current->line);
        current = current->prev;
        free(current->next);
        current->next = NULL;
    }
    return 0;//no error
}

void signal_handler(int signum){ //for Ctrl+C to not close the program directly
    system("clear");
    printf(".....Exiting and saving changes......\nPress any key to confirm.\n");
    fflush(stdout);
    Exit = True;
}

int editor(node* buffer){
    node* current = buffer;
    int list_length;
    int len;
    char new_char;
    signal(SIGINT, signal_handler);//Ctrl+C handeling
    while(!Exit){//app logic
        current = go_to(buffer,CURSOR.row + 1);
        list_length = display_buffer(buffer);
        printf("Use arrow keys to navigate, Ctrl+C to quit by saving into file, Ctrl+Z to quit without saving.\n");
        // to take char and handle it
        new_char = getch();
        if (new_char == '\033') { // Escape sequence for arrow keys
            char next = getch();
            if (next == '[') { // Arrow keys
                switch (getch()) {
                    case 'A': // Up
                        if (current->prev != NULL) {
                            current = current->prev;
                            CURSOR.row--;
                        }
                        break;
                    case 'B': // Down
                        if (current->next != NULL) {
                            current = current->next;
                            CURSOR.row++;
                        }
                        break;
                    case 'C': // Right
                        if(strlen(current->line) != CURSOR.column){
                            CURSOR.column++;
                        }
                        else if (current->next != NULL){//go to next line if cursor at end of current line
                            current = current->next;
                            CURSOR.row++;
                            CURSOR.column = 0;
                        }
                        break;
                    case 'D': // Left
                        if(0 != CURSOR.column){
                            if ((len = (strlen(current->line))) < CURSOR.column)
                                CURSOR.column = len;
                            CURSOR.column--;
                        }else { //go to previous line if cursor at start of current line 
                            if(CURSOR.row !=0){
                                current = current->prev;
                                CURSOR.row--;
                                if (current->line != NULL) {
                                    CURSOR.column = strlen(current->line);
                                } else {
                                    CURSOR.column = 0;
                                }
                            }
                        }
                        break;
                        case '3': //delete key
                        if (getch() == '~') {
                            int return_value = delete(current);
                            if (return_value != 0) {
                                return 1;
                            }
                        }
                        break;
                }
            }
        }
        else if(Exit) break;//to exit and save file on ctrl+C
        else if(new_char == 127){ //backspace key
            int return_value = backspace(current);
            if(return_value != 0){
                return 1;
            }
        }
        else{//add the char to the buffer if not special keys
            int return_value = update_buffer(buffer,new_char);
            if(return_value != 0){
                return 1;
            }
        }
    }
    return 0;
}

int delete(node* current){
    int len = strlen(current->line);
    if(len > CURSOR.column){//for cursor not at end of the line
        for(int i = CURSOR.column; i < len; i++){
            *(current->line + i) = *(current->line + i + 1);
        }
        current->line = (string)realloc(current->line,len);
        if(current->line == NULL){
            return 1;
        }
    }
    else {//for cursor at end of the line (delete newline character)
        if (current->next != NULL){
            current = current->next;
            CURSOR.row++;
            CURSOR.column = 0;
            int return_value = backspace(current);//the logic is handeled in backspace function
            if(return_value != 0){
                return 1;
            }
        }
    }
    return 0;
}

int backspace(node* current){
    if(CURSOR.column!=0){//for cursor not at start of the line
        CURSOR.column--;
        int return_value = delete(current);//backspace is same as delete with only different cursor position
        if(return_value != 0){
            return 1;
        }
    }
    else if(CURSOR.row != 0){ //for cursor at start of line
        //deleting newline character
        int len_prev = strlen(current->prev->line);
        int len_curr = strlen(current->line);
        current->prev->line = (string)realloc(current->prev->line,len_prev + len_curr + 1);//reallocate memory enough to store 2 merged lines
        if(current->prev->line == NULL){
            return 1;
        }

        strcat(current->prev->line,current->line); //merge 2 lines
        //disconnect later line from others
        if (current->prev != NULL) {
            current->prev->next = current->next;
        }
        if (current->next != NULL) {
            current->next->prev = current->prev;
        }
        
        node* old = current;

        // Move cursor to previous line
        current = current->prev;
        CURSOR.row--;
        CURSOR.column = len_prev;

        //free and remove later line
        free(old->line);
        old->line = NULL;
        old->next = NULL;
        old->prev = NULL;
        free(old);   
    }
    return 0;
}

int display_buffer(node* head){
    system("clear");//new screen
    node* current = head;
    int length;
    for(length = 0;current != NULL;length++){
        printf("%d\t",length+1);
        if (CURSOR.row == length){//add cursor character at cursor position
            for(int i = 0;*(current->line+i) != '\0';i++){//cursor within the line
                if(CURSOR.column == i)
                    print_cursor(*(current->line+i));
                else
                    printf("%c",*(current->line+i));
            }
            if(CURSOR.column >= strlen(current->line))//cursor at the end of line(to add characters at end of line)
                print_cursor(' ');
            printf("\n");
        }
        else //print the line without cursor
            printf("%s\n", current->line);
        current = current->next;
    }
    return length; //no of lines in the buffer currently
}

int update_buffer(node* head,char new_char){//adding char to buffer
    node* current = go_to(head, CURSOR.row+1);
    int len = strlen(current->line);
    //increase length of current line for new character
    current->line = (string)realloc(current->line,(len+2)*sizeof(char));
    if(current->line == NULL){
        return 1;
    }
    len++;
    for(int i = len;i >=CURSOR.column; i--){
        *(current->line + i) = *(current->line + i-1);
    }
    *(current->line + CURSOR.column) = new_char;//add new char
    CURSOR.column++;// moving cursor 1 position after character adding
    if(new_char == '\n'){//if Enter pressed add a new line
        int i;
        node* new = create_node();//create a node for added new line
        if(new == NULL){
            return 1;
        }
        //initialize new line
        new->line = (string)malloc(1);
        if(new->line == NULL)
            return 1;
        new->line[0] = '\0';
        //create new line and adjust the line contents(break the line on newline character to 2 separate lines)
        int found_newline = False;
        int len = strlen(current->line);
        for(i = 0;i <= len; i++){
            if(*(current->line+i) == '\n'){
                found_newline = i+1;
                *(current->line+i) = '\0';
            }
            else if(found_newline){
                new->line = (string)realloc(new->line,strlen(new->line)+1);
                *(new->line+i-found_newline) = *(current->line+i);
                if(new->line == NULL){
                    return 1;
                }
            }
        }
        string temp = (string)realloc(current->line, strlen(current->line) + 1);
        if (temp == NULL) {
            return 1;
        }
        current->line = temp;
        //link this new line in buffer
        new->next = current->next;
        new->prev = current;
        if(current->next != NULL)
            current->next->prev = new;
        current->next = new;
        //adjust cursor position
        CURSOR.row++;
        CURSOR.column = 0;
    }
    return 0;
}

node* go_to(node* head, int line_no){
    node* current = head;
    for(int i = 1; i < line_no; i++){
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
    node* temp;
    while (current != NULL) {
        temp = current->next;
        if (current->line != NULL) {
            free(current->line);
        }
        free(current);
        current = temp;
    }
    return 0;
}
