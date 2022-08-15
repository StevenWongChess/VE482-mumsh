#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMD_MAX_LENGTH 1024

typedef struct command_data{
    int size;
    char **vector; 
} command_data;

typedef struct line_data{
    int size;
    command_data *vector;
    // bool incomplete;
} line_data;

void print_prompt1(void){
    printf("mum $ ");
}
void print_prompt2(void){
    printf(" > ");
}
void garbage_collect(line_data* l){
    int n = l->vector[0].size;
    for(int i = 0; i < n; ++i){
        free(l->vector[0].vector[i]);
    }
    free(l->vector[0].vector);
    free(l->vector);
    free(l);
}

void arg_push_back(line_data *l, char arg[50], int *it){
    l->vector[0].vector[l->vector[0].size] = malloc(50 * sizeof(char));
    strcpy(l->vector[0].vector[l->vector[0].size], arg);
    ++(l->vector[0].size);
    arg[0] = '\0';
    *it = 0;
}

void execute(command_data *command){
    pid_t pid;
    int status;

    

    pid = fork();

    if(pid < 0){
        printf("fail to fork\n");
    }
    else if(pid == 0){
        if (execvp(command->vector[0], command->vector) < 0){
            printf("%s: command not found\n", command->vector[0]);
            exit(EXIT_FAILURE);
        }
    }
    else{
        waitpid(pid, &status, 0);
    }
}

line_data *read_line(){
    char buffer[CMD_MAX_LENGTH];
    line_data *l = NULL;

    if(fgets(buffer, CMD_MAX_LENGTH, stdin)){
        int buffer_size = strlen(buffer);

        // decide if there is multi-line case
        if(!l){
            l = malloc(sizeof(line_data));
            l->size = 0;
            l->vector = malloc(sizeof(command_data));
            l->vector[0].size = 0;
            l->vector[0].vector = malloc(20 * sizeof(char*));
        }

        char arg[50] = "\0"; 
        int it = 0; 
        bool prev_blank = true;

        for(int i = 0; i < buffer_size - 1; ++i){
            if(buffer[i] == ' '){
                if(prev_blank)
                    continue;
                else{
                    prev_blank = true;
                    if(arg[0] != '\0'){
                        arg_push_back(l, arg, &it);
                    }
                }
            }
            else if(buffer[i] == '<'){
                if(!prev_blank){
                    // push last arg first
                    arg_push_back(l, arg, &it);
                    prev_blank = true;
                }
                // then push < 
                arg[0] = '<'; arg[1] = '\0'; it = 1;
                arg_push_back(l, arg, &it);
            }
            else if(buffer[i] == '>'){
                if(i < buffer_size - 2 && buffer[i+1] == '>'){
                    if(!prev_blank){
                        arg_push_back(l, arg, &it);
                        prev_blank = true;
                    }
                    arg[0] = '>'; arg[1] = '>'; arg[2] = '\0'; it = 2;
                    arg_push_back(l, arg, &it);
                    ++i;
                }
                else{
                    if(!prev_blank){
                        arg_push_back(l, arg, &it);
                        prev_blank = true;
                    }
                    arg[0] = '>'; arg[1] = '\0'; it = 1;
                    arg_push_back(l, arg, &it);
                }
            }
            else{
                prev_blank = false;
                arg[it] = buffer[i];
                ++it;	
                arg[it] = '\0';
            }
        }
        if(!prev_blank){
            arg_push_back(l, arg, &it);
        }
        // if(incomplete){
        //     continue;
        // }
        // break;
    }

    return l;
}

int main(){

    line_data *l = NULL;

    do{
        print_prompt1();
        l = read_line();
        // check if input reading is correct
        // for(int i = 0; i < l->vector[0].size; ++i){
        //     printf("%s\n", l->vector[0].vector[i]);
        // }
        if(l->vector[0].size == 1 && strncmp(l->vector[0].vector[0], "exit", 4) == 0){
            garbage_collect(l);
            break;
        }
        execute(&(l->vector[0]));
        garbage_collect(l);
    }while(true);
    
    exit(EXIT_SUCCESS);
}

