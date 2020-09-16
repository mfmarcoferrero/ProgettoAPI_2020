#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 20
#define MAX_LINE_LENGTH 1025

//-----------------------------------------------------------//
//------------------------TYPEDEF----------------------------//
//-----------------------------------------------------------//

typedef enum {
    CHANGE = 0, DELETE = 1, PRINT = 2, UNDO = 3, REDO = 4, QUIT = 5, DEFAULT = 6
} command;

typedef enum { false, true } boolean;

typedef struct line * line_pointer;
typedef struct history_elem * history_elem_pointer;
typedef line_pointer * text_array_pointer;
typedef history_elem_pointer * history_pointer;

//-----------------------------------------------------------//
//--------------------GLOBAL VARIABLES-----------------------//
//-----------------------------------------------------------//

struct line{
    int line_number;
    char * text;
};

struct history_elem{
    command type;
    int first_line;
    int last_line;
    int text_array_history_dim;
    text_array_pointer text_array_history;
};

char command_buffer[MAX_COMMAND_LENGTH];
char command_type;
int command_len;
int first_line;
int last_line;
int undo_number;
int redo_number;
int total_u_r_num = 0;
int current_history_ind = -1;
text_array_pointer text_array = NULL;
history_pointer history_array = NULL;
int text_array_dim = 0;
int history_array_dim = 0;

command parse_command();
void handle_CHANGE();
void handle_DELETE();
void handle_UNDO();
void handle_REDO();
void handle_QUIT();

//-----------------------------------------------------------//
//----------------------DYNAMIC ARRAY------------------------//
//-----------------------------------------------------------//


text_array_pointer new_text_elem(text_array_pointer array,int ind, int new_line_num){
    char * buffer[MAX_LINE_LENGTH];
    array[ind] = malloc(sizeof(struct line));
    fgets(buffer,MAX_LINE_LENGTH,stdin);
    array[ind] -> text = (char *) malloc((strlen(buffer) * sizeof(char)) + 1);
    strcpy(array[ind] -> text, buffer);
    array[ind] -> line_number = new_line_num;
}

void add_delete_to_history(int first_line,int last_line){
    if (history_array == NULL){
        history_array = malloc(sizeof(history_pointer));
    } else {
        history_array = realloc(history_array,(history_array_dim + 1) * sizeof(history_pointer));
    }
    history_array[history_array_dim] = malloc(sizeof(struct history_elem));
    history_array[history_array_dim] -> type = DELETE;
    history_array[history_array_dim] -> first_line = first_line;
    history_array[history_array_dim] -> last_line = last_line;
    history_array[history_array_dim] -> text_array_history_dim = 0;
    history_array[history_array_dim] -> text_array_history = NULL;
    history_array_dim++;
}

void add_change_to_history(int first_line, int last_line) {
    if (history_array == NULL){
        history_array = malloc(sizeof(history_pointer));
    } else {
        history_array = realloc(history_array,(history_array_dim + 1) * sizeof(history_pointer));
    }
    history_array[history_array_dim] = malloc(sizeof(struct history_elem));
    history_array[history_array_dim] -> type = CHANGE;
    history_array[history_array_dim] -> first_line = first_line;
    history_array[history_array_dim] -> last_line = last_line;
    history_array[history_array_dim] -> text_array_history_dim = 0;
    history_array[history_array_dim] -> text_array_history = NULL;
    history_array_dim++;
}

text_array_pointer add_line(text_array_pointer array, int key, int ind){
    if (ind == -1){
        if (array == NULL){
            array = malloc(sizeof(line_pointer));
        } else {
            array = realloc(array,(text_array_dim + 1) * sizeof(line_pointer));
        }
        new_text_elem(array,text_array_dim,key);
        text_array_dim++;
    } else {
        new_text_elem(array,ind,key);
    }
    return array;
}

text_array_pointer left_shift_block (text_array_pointer array, int array_dim, int block_dim, int first_elem_ind){
    if (array_dim - block_dim == 0){
        array = NULL;
    } else {
        for (int i = first_elem_ind; i + block_dim < array_dim; i++) {
            array[i] = array[i + block_dim];
            array[i]->line_number = array[i]->line_number - block_dim;
        }
        array = realloc(array,(array_dim - block_dim) * sizeof(line_pointer));
    }
    return array;
}

void right_shift_block (int block_dim, int first_elem_ind, int history_elem_ind){
    text_array = realloc(text_array, (text_array_dim + block_dim) * sizeof(line_pointer));
    text_array_dim = text_array_dim + block_dim;
    for (int i = text_array_dim - 1; i >= first_elem_ind; i--) {
        if (i > first_elem_ind + block_dim - 1) {
            text_array[i] = text_array[i - block_dim];
            text_array[i] -> line_number = i + 1;
        } else {
            text_array[i] = history_array[history_elem_ind] -> text_array_history[history_array[history_elem_ind] -> text_array_history_dim - 1];
            history_array[history_elem_ind] -> text_array_history[history_array[history_elem_ind] -> text_array_history_dim - 1] = NULL;
            history_array[history_elem_ind] -> text_array_history_dim--;
        }
    }
    history_array[history_elem_ind] -> text_array_history = NULL;
}

text_array_pointer delete_lines_and_save_history(text_array_pointer array, int array_len, int first_line_delete, int last_line_delete){
    int lines_block_dim = 0;
    int first = first_line_delete;
    int last;
    boolean valid = true;
    if (array == NULL || first_line_delete > array[array_len - 1] -> line_number){
        valid = false;
        add_delete_to_history(-1,-1);
    }
    if (valid == true){
        if (last_line_delete > array[array_len - 1] -> line_number){
            last = array[array_len - 1] -> line_number;
        } else {
            last = last_line_delete;
        }
        add_delete_to_history(first_line_delete,last);
        while (first <= last){
            if (history_array[history_array_dim - 1] -> text_array_history == NULL){
                history_array[history_array_dim - 1] -> text_array_history = malloc(sizeof(line_pointer));
            } else {
                history_array[history_array_dim - 1] -> text_array_history = realloc(history_array[history_array_dim - 1] -> text_array_history, (history_array[history_array_dim - 1] -> text_array_history_dim + 1) * sizeof(line_pointer));
            }
            history_array[history_array_dim - 1] -> text_array_history[history_array[history_array_dim - 1] -> text_array_history_dim] = text_array[first - 1];
            history_array[history_array_dim - 1] -> text_array_history_dim++;
            array[first - 1] = NULL;
            lines_block_dim++;
            first++;
        }
        array = left_shift_block(array, array_len, lines_block_dim, first_line_delete - 1);
        text_array_dim = text_array_dim - lines_block_dim;
    }
    return array;
}

void print_lines(text_array_pointer array, int array_len, int first, int last){
    if (first <= last) {
        if (first > text_array_dim || first == 0) {
            fputs(".\n", stdout);
            print_lines(array, array_len, first + 1, last);
        } else {
            fputs(array[first - 1] -> text, stdout);
            print_lines(array, array_len, first + 1, last);
        }
    }
}

text_array_pointer kill_text_array(){
    for (int i = 0; i < text_array_dim; i++) {
        if (text_array[i] != NULL) {
            free(text_array[i]);
            text_array[i] = NULL;
        }
    }
    return NULL;
}

history_pointer kill_history_array(){
    for (int i = 0; i < history_array_dim; i++) {
        if (history_array[i] != NULL) {
            free(history_array[i]);
            history_array[i] = NULL;
        }
    }
    history_array_dim = 0;
    current_history_ind = -1;
    return NULL;
}

void print_text_array(text_array_pointer array,int dim){
    if (array == NULL){
        printf("T_A = NULL\n");
    } else {
        printf("TEXT ARRAY DIM: %d\n",dim);
        for (int i = 0; i < dim; i++) {
            if (array[i] == NULL) {
                printf("T_A[%d]: NULL\n", i);
            } else
                printf("T_A[%d]: Line -> %d  Text -> %s", i, array[i]->line_number, array[i]->text);
        }
    }
}

void print_history_array(history_pointer array,int dim){
    printf("--------------HISTORY ARRAY-------------\n");
    if (array == NULL){
        printf("H_A = NULL\n");
    } else {
        printf("HISTORY ARRAY DIM = %d\n",history_array_dim);
        printf("H_A CURRENT IND = %d\n",current_history_ind);
        for (int i = 0; i < dim; i++) {
            if (array[i] == NULL) {
                printf("H_A[%d]: NULL\n\n", i);
            } else if (array[i]->type == DELETE) {
                printf("H_A[%d]:DELETE  First_Line -> %d  Last_Line -> %d\n", i, array[i]->first_line,
                       array[i]->last_line);
            } else if (array[i] -> type == CHANGE) {
                printf("H_A[%d]:CHANGE  First_Line -> %d  Last_Line -> %d\n", i, array[i]->first_line,
                       array[i]->last_line);
            }
            print_text_array(array[i] -> text_array_history, array[i] -> text_array_history_dim);
        }
    }
}

void save_text_lines_in_history(int first_text_line,int last_text_line,int history_elem_ind){
    int first = first_text_line, last = last_text_line, i = 0;
    if (history_array[history_elem_ind] -> text_array_history == NULL){
        history_array[history_elem_ind] -> text_array_history = malloc(sizeof(line_pointer) * (last_text_line - first_text_line + 1));
    } else {
        history_array[history_elem_ind] -> text_array_history = realloc(history_array[history_elem_ind] -> text_array_history, (history_array[history_elem_ind] -> text_array_history_dim + last_text_line - first_text_line + 1) * sizeof(line_pointer));
    }
    while (first <= last){
        history_array[history_elem_ind] -> text_array_history[history_array[history_elem_ind] -> text_array_history_dim] = text_array[first - 1];
        text_array[first - 1] = NULL;
        history_array[history_elem_ind] -> text_array_history_dim++;
        first++;
    }
    text_array = left_shift_block(text_array,text_array_dim,last_text_line - first_text_line + 1,first_text_line - 1);
    text_array_dim = text_array_dim - (last_text_line - first_text_line + 1);
}

void play_undo(int undo_num) {
    line_pointer temp;
    if (history_array != NULL && undo_num > 0) {
        for (int i = 0; i < undo_num; i++) {
            if (history_array[current_history_ind] -> type == DELETE) {
                if (history_array[current_history_ind] -> first_line != -1 && history_array[current_history_ind] -> last_line != -1) {
                    right_shift_block(history_array[current_history_ind]->text_array_history_dim,
                                      history_array[current_history_ind]->text_array_history[0]->line_number - 1,
                                      current_history_ind);
                }
            } else if (history_array[current_history_ind] -> type == CHANGE) {
                if (history_array[current_history_ind] -> text_array_history == NULL){
                    save_text_lines_in_history(history_array[current_history_ind] -> first_line,history_array[current_history_ind] -> last_line,current_history_ind);
                } else {
                    int first_line_overwrite = history_array[current_history_ind] -> first_line;
                    int last_line_overwrite = history_array[current_history_ind]->text_array_history[
                            history_array[current_history_ind] -> text_array_history_dim - 1] -> line_number;
                    int last_line_history = history_array[current_history_ind] -> last_line;
                    int j = 0;
                    while (first_line_overwrite <= last_line_overwrite) {
                        temp = text_array[first_line_overwrite - 1];
                        text_array[first_line_overwrite - 1] = history_array[current_history_ind] -> text_array_history[j];
                        history_array[current_history_ind] -> text_array_history[j] = temp;
                        temp = NULL;
                        first_line_overwrite++;
                        j++;
                    }
                    if (last_line_overwrite < last_line_history){
                        save_text_lines_in_history(last_line_overwrite + 1,last_line_history,current_history_ind);
                    }
                }
            }
            current_history_ind--;
        }
    }
}
void save_history_lines_in_text(int first_text_line,int last_text_line,int history_text_ind){
    int first = first_text_line, last = last_text_line, i = history_text_ind;
    if (text_array == NULL){
        text_array = malloc(sizeof(line_pointer) * (last_text_line - first_text_line + 1));
    } else {
        text_array = realloc(text_array, (text_array_dim + last_text_line - first_text_line + 1) * sizeof(line_pointer));
    }
    while (first <= last){
        text_array[first - 1] = history_array[current_history_ind] -> text_array_history[i];
        history_array[current_history_ind] -> text_array_history[i] = NULL;
        text_array_dim++;
        first++;
        i++;
    }
    history_array[current_history_ind] -> text_array_history = left_shift_block(history_array[current_history_ind] -> text_array_history,history_array[current_history_ind] -> text_array_history_dim,last_text_line - first_text_line + 1,history_text_ind);
    history_array[current_history_ind] -> text_array_history_dim = history_array[current_history_ind] -> text_array_history_dim - (last_text_line - first_text_line + 1);
}

void play_redo(redo_num){
    line_pointer temp;
    if (history_array != NULL && redo_num > 0) {
        for (int i = 0; i < redo_num; i++) {
            current_history_ind++;
            if (history_array[current_history_ind] -> type == DELETE) {
                if (history_array[current_history_ind] -> first_line != -1 && history_array[current_history_ind] -> last_line != -1){
                    save_text_lines_in_history(history_array[current_history_ind] -> first_line, history_array[current_history_ind] -> last_line, current_history_ind);
                }
            } else if (history_array[current_history_ind] -> type == CHANGE) {
                boolean overwrite = false;
                int first_line_overwrite;
                int last_line_overwrite;
                int first_line_history = history_array[current_history_ind] -> first_line;
                int last_line_history = history_array[current_history_ind] -> last_line;
                if (text_array != NULL && history_array[current_history_ind] -> first_line <= text_array[text_array_dim - 1] -> line_number){
                    overwrite = true;
                    first_line_overwrite = history_array[current_history_ind] -> first_line;
                    if (history_array[current_history_ind] -> last_line <= text_array[text_array_dim - 1] -> line_number){
                        last_line_overwrite = history_array[current_history_ind] -> last_line;
                    } else {
                        last_line_overwrite = text_array[text_array_dim - 1] -> line_number;
                    }
                }
                int j = 0;
                if (overwrite){
                    while (first_line_overwrite <= last_line_overwrite) {
                        temp = text_array[first_line_overwrite - 1];
                        text_array[first_line_overwrite - 1] = history_array[current_history_ind]->text_array_history[j];
                        history_array[current_history_ind]->text_array_history[j] = temp;
                        temp = NULL;
                        first_line_overwrite++;
                        j++;
                    }
                    if (last_line_overwrite < last_line_history){
                        save_history_lines_in_text(first_line_overwrite,last_line_history,j);
                    }
                } else {
                    save_history_lines_in_text(first_line_history,last_line_history,0);
                }
            }
        }
    }
}


//-----------------------------------------------------------//
//---------------------HANDLE COMMAND------------------------//
//-----------------------------------------------------------//

void print_all(){
    printf("--------------TEXT ARRAY-------------\n");
    print_text_array(text_array,text_array_dim);
    print_history_array(history_array,history_array_dim);
}

void handle_CHANGE(){
    int i = first_line;
    add_change_to_history(first_line,last_line);
    while (i <= last_line){
        boolean overwrite = false;
        if (i <= text_array_dim){
            overwrite = true;
            if (history_array[history_array_dim - 1] -> text_array_history == NULL){
                history_array[history_array_dim - 1] -> text_array_history = malloc(sizeof(line_pointer));
            } else {
                history_array[history_array_dim - 1] -> text_array_history = realloc(history_array[history_array_dim - 1] -> text_array_history, (history_array[history_array_dim - 1] -> text_array_history_dim + 1) * sizeof(line_pointer));
            }
            history_array[history_array_dim - 1] -> text_array_history[history_array[history_array_dim - 1] -> text_array_history_dim] = text_array[i - 1];
            history_array[history_array_dim - 1] -> text_array_history_dim++;
        }
        if (overwrite == true) {
            text_array = add_line(text_array, i, i - 1);
        } else {
            text_array = add_line(text_array,i,-1);
        }
        i++;
    }
    current_history_ind++;
    //print_all();
}

void handle_DELETE(){
    text_array = delete_lines_and_save_history(text_array,text_array_dim,first_line,last_line);
    current_history_ind++;
    //print_all();
}

void handle_PRINT(){
    print_lines(text_array,text_array_dim,first_line,last_line);
}

void cut_history_array(int ind){
    int last_ind = history_array_dim;
    for (int i = ind; i < last_ind; i++) {
        if (history_array[i] != NULL) {
            history_array[i] = NULL;
        }
        history_array_dim--;
    }
    history_array = realloc(history_array,(history_array_dim) * sizeof(history_pointer));
}

void handle_UNDO(){
    command next_command = UNDO;
    do {
        int max_undo_num = current_history_ind + 1;
        int max_redo_num = history_array_dim - (current_history_ind + 1);
        total_u_r_num = 0;
        do {
            if (next_command == UNDO){
                if (total_u_r_num - undo_number + max_undo_num > 0){
                    total_u_r_num = total_u_r_num - undo_number;
                } else {
                    total_u_r_num = 0 - max_undo_num;
                }
            } else if (next_command == REDO){
                if (total_u_r_num + redo_number < max_redo_num) {
                    total_u_r_num = total_u_r_num + redo_number;
                } else {
                    total_u_r_num = max_redo_num;
                }
            }
            //printf("TOTAL UNDO/REDO NUM: %d\n",total_u_r_num);
            //printf("MAX NUM UNDO: %d\n",max_undo_num);
            //printf("MAX NUM REDO: %d\n",max_redo_num);
            next_command = parse_command();
        } while (next_command == UNDO || next_command == REDO);
        if (total_u_r_num < 0){
            play_undo(0 - total_u_r_num);
        } else if (total_u_r_num > 0){
            play_redo(total_u_r_num);
        }
        max_undo_num = max_undo_num + total_u_r_num;
        max_redo_num = max_redo_num - total_u_r_num;
        if (next_command == PRINT){
            //print_all();
            handle_PRINT();
        }
    } while (next_command == PRINT);

    if (next_command == CHANGE){
        cut_history_array(current_history_ind + 1);
        handle_CHANGE();
    } else if (next_command == DELETE){
        cut_history_array(current_history_ind + 1);
        handle_DELETE();
    } else if (next_command == QUIT){
        handle_QUIT();
    }
}

void handle_QUIT(){
    //text_array = kill_text_array();
    //history_array = kill_history_array();
    exit(0);
}


//-----------------------------------------------------------//
//----------------------PARSE COMMAND------------------------//
//-----------------------------------------------------------//

void save_border_lines(){
    char * token;
    token = strtok(command_buffer, ",");
    first_line = atoi(token);
    token = strtok(NULL, ",");
    last_line = atoi(token);
}

command parse_command(){
    command_len = (int) strlen(fgets(command_buffer,MAX_COMMAND_LENGTH,stdin));
    command_type = command_buffer[command_len - 2];
    command_buffer[command_len - 2] = '\0';
    if (command_type == 'u'){
        undo_number = atoi(command_buffer);
        return UNDO;
    } else if (command_type == 'r'){
        redo_number = atoi(command_buffer);
        return REDO;
    } else if (command_type == 'c') {
        save_border_lines();
        return CHANGE;
    } else if (command_type == 'd') {
        save_border_lines();
        return DELETE;
    } else if (command_type == 'p') {
        save_border_lines();
        return PRINT;
    } else if (command_type == 'q') {
        return QUIT;
    } else
        return DEFAULT;
}

boolean handle_command() {
    command command_chosen = parse_command();
    switch (command_chosen) {
        case CHANGE:
            handle_CHANGE();
            return false;
        case DELETE:
            handle_DELETE();
            return false;
        case PRINT:
            handle_PRINT();
            return false;
        case UNDO:
            handle_UNDO();
            return false;
        case REDO:
            //handle_REDO();
            return false;
        case QUIT:
            handle_QUIT();
            return true;
        case DEFAULT:
            return false;
    }
}

int main() {
    boolean stop = false;
    do {
        stop = handle_command();
    } while (!stop);
    return 0;
}
