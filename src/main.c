#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>

#define CMD_MAX_LENGTH 1024
#define ARG_MAX_LENGTH 50
#define PWD_MAX_LENGTH 50
#define TOKENS_INITIAL_CAPACITY 4
#define TOKENS_RESIZE_FACTOR 2
char OLDPWD[PWD_MAX_LENGTH] = "\0";

static sigjmp_buf env;
static volatile sig_atomic_t isexecuting = 0;
void sigint_handler(){
    if(isexecuting == 1){
        printf("haha\n");
        return;
    }
    else{
        printf("laaa\n");
        siglongjmp(env, 42);
    }
}

// define data structures here
typedef struct Tokens{
    // will resize like a std::vector in c++14
    int size;
    int capacity;
    char **vector;
}Tokens;

typedef struct Command{
    int size;
    char **vector; 
}Command;

typedef struct Command_table{
    int size;
    Command *vector;
    char infd[ARG_MAX_LENGTH];
    char appfd[ARG_MAX_LENGTH];
    char owfd[ARG_MAX_LENGTH];
}Command_table;

void print_prompt1(void){
    printf("mum $ ");
}
void print_prompt2(void){
    printf(" > ");
}
void print_tokens(Tokens* tokens){
    printf("----printing tokens----\n");
    for(int i=0; i < tokens->size; i++){
        printf("%s\n", tokens->vector[i]);
    }
}
void print_table(Command_table* table){
    printf("----printing table----\n");
    for(int i = 0; i < table->size; ++i){
        for(int j = 0; j < table->vector[i].size; ++j){
            printf("%s ", table->vector[i].vector[j]);
        }
        printf("\n");
    }
}
void garbage_collect(Tokens* tokens, Command_table* table){
    if(!tokens)
        return;

    for(int i = 0; i < tokens->size; ++i){
        free(tokens->vector[i]);
    }
    free(tokens->vector);
    free(tokens);

    if(!table)
        return;

    for(int i = 0; i < table->size; ++i){
        for(int j = 0; j < table->vector[i].size; ++j){
            if(table->vector[i].vector[j] != NULL)
                free(table->vector[i].vector[j]);
        }
        free(table->vector[i].vector);
    }
    free(table->vector); 
    free(table);
}

void token_push_back(Tokens* tokens, char token_buffer[ARG_MAX_LENGTH], int *it){
    if(tokens->size == tokens->capacity){
        tokens->capacity *= TOKENS_RESIZE_FACTOR;
        tokens->vector = realloc(tokens->vector, tokens->capacity * sizeof(char*));
    }
    tokens->vector[tokens->size] = malloc(ARG_MAX_LENGTH * sizeof(char));
    strcpy(tokens->vector[tokens->size], token_buffer);
    ++(tokens->size);
    token_buffer[0] = '\0';
    *it = 0;
}

Tokens* read_line(){
    print_prompt1();
    char buffer[CMD_MAX_LENGTH];
    Tokens *tokens = NULL;

    if(fgets(buffer, CMD_MAX_LENGTH, stdin)){
        int buffer_size = strlen(buffer);

        // decide if there is multi-line case
        if(!tokens){
            tokens = malloc(sizeof(Tokens));
            tokens->size = 0;
            tokens->capacity = TOKENS_INITIAL_CAPACITY;
            tokens->vector = malloc(TOKENS_INITIAL_CAPACITY * sizeof(char*));
        }

        char token_buffer[ARG_MAX_LENGTH] = "\0"; 
        int it = 0; 
        bool prev_blank = true;

        // iterate every char
        for(int i = 0; i < buffer_size; ++i){

            if(buffer[i] == ' '){
                if(!prev_blank){
                    prev_blank = true;
                    token_push_back(tokens, token_buffer, &it);
                }
            }
            else if(buffer[i] == '<' || buffer[i] == '|'){
                if(!prev_blank){
                    prev_blank = true;
                    token_push_back(tokens, token_buffer, &it);
                }
                token_buffer[0] = buffer[i]; token_buffer[1] = '\0'; it = 1;
                token_push_back(tokens, token_buffer, &it);
            }
            else if(buffer[i] == '>'){
                if(i < buffer_size - 2 && buffer[i + 1] == '>'){
                    // handle >> case
                    if(!prev_blank){
                        prev_blank = true;
                        token_push_back(tokens, token_buffer, &it);
                    }
                    token_buffer[0] = '>'; token_buffer[1] = '>'; token_buffer[2] = '\0'; it = 2;
                    token_push_back(tokens, token_buffer, &it);
                    ++i;
                }
                else{
                    // handle > case
                    if(!prev_blank){
                        prev_blank = true;
                        token_push_back(tokens, token_buffer, &it);
                    }
                    token_buffer[0] = '>'; token_buffer[1] = '\0'; it = 1;
                    token_push_back(tokens, token_buffer, &it);
                }
            }
            else if(buffer[i] == '\n'){
                break;
            }
            else{
                prev_blank = false;
                token_buffer[it] = buffer[i];
                ++it;	
                token_buffer[it] = '\0';
            }
        }

        if(!prev_blank){
            prev_blank = true;
            token_push_back(tokens, token_buffer, &it);
        }
    }

    // mimic the behavior of sh, 
    // for bash, double ctrl d should do nothing, 
    // use feof(stdin) && token != NULL => should continue reading
    if(feof(stdin)){
        Command_table* table = NULL;
        garbage_collect(tokens, table);
        printf("\nexit\n");
        exit(EXIT_SUCCESS);
    }

    return tokens;
}

Command_table* parse(Tokens* tokens){
    Command_table* table = malloc(sizeof(Command_table));
    table->infd[0] = table->appfd[0] = table->owfd[0] = '\0';
    table->size = 1;
    table->vector = malloc(sizeof(Command));
    table->vector[table->size - 1].size = 0;
    table->vector[table->size - 1].vector = realloc(NULL, sizeof(char*));
    table->vector[table->size - 1].vector[0] = NULL;

    for(int i = 0; i < tokens->size; ++i){
        if(strcmp(tokens->vector[i], ">") == 0){
            strcpy(table->owfd, tokens->vector[i + 1]);
            ++i;
        }
        else if(strcmp(tokens->vector[i], ">>") == 0){
            strcpy(table->appfd, tokens->vector[i + 1]);
            ++i;
        }
        else if(strcmp(tokens->vector[i], "<") == 0){
            strcpy(table->infd, tokens->vector[i + 1]);
            ++i;
        }
        else if(strcmp(tokens->vector[i], "|") == 0){
            ++(table->size);
            table->vector = realloc(table->vector, table->size * sizeof(Command));
            table->vector[table->size - 1].size = 0;
            table->vector[table->size - 1].vector = realloc(NULL, sizeof(char*));
            table->vector[table->size - 1].vector[0] = NULL;
        }
        else{
            Command* c = &(table->vector[table->size - 1]);
            ++(c->size);
            c->vector = realloc(c->vector, (c->size + 1) * sizeof(char*));
            c->vector[c->size - 1] = malloc(sizeof(char) * ARG_MAX_LENGTH);
            strcpy(c->vector[c->size - 1], tokens->vector[i]);
            c->vector[c->size] = NULL;
        }
    }
    return table;
}

void execute(Command_table* table){
    // store stdin & stdout and restore later
    int savein = dup(STDIN_FILENO);
    int saveout = dup(STDOUT_FILENO);
    int infd, outfd, pid;
    if(strlen(table->infd) > 0){
        infd = open(table->infd, O_RDONLY, 0644);
    } 
    else{
        infd = dup(savein);
    }

    for(int i = 0; i < table->size; ++i){
        // Handle Ctrl-C for child processes
        struct sigaction act;
        act.sa_handler = sigint_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_RESTART;
        sigaction(SIGINT, &act, NULL);

        dup2(infd, STDIN_FILENO);
        close(infd);

        if(i == table->size - 1){
            // last command: need to deal with output redirection
            if(strlen(table->owfd) > 0){
                outfd = open(table->owfd, O_WRONLY|O_CREAT|O_TRUNC, 0644);
            }
            else if(strlen(table->appfd) > 0){
                outfd = open(table->appfd, O_WRONLY|O_CREAT|O_APPEND, 0644);
            }
            else{
                outfd = dup(saveout);
            }
        }
        else{
            // other commands, do pipe
            int fd[2];
            pipe(fd);
            outfd = fd[1];
            infd = fd[0];
        }

        dup2(outfd, STDOUT_FILENO);
        close(outfd);

        // built in command
        if(strcmp(table->vector[i].vector[0], "cd") == 0){
            char current_wd[PWD_MAX_LENGTH];
            getcwd(current_wd, PWD_MAX_LENGTH);
            if(table->vector[i].size <= 1 || strcmp(table->vector[i].vector[1], "~") == 0){
                chdir(getenv("HOME"));
                strcpy(OLDPWD, current_wd);
            }
            else if(strcmp(table->vector[i].vector[1], "-") == 0){
                if(strlen(OLDPWD) == 0){
                    printf("bash: cd: OLDPWD not set\n");
                }
                else{
                    chdir(OLDPWD);
                    strcpy(OLDPWD, current_wd);
                }
            }
            else{
                if(chdir(table->vector[i].vector[1]) == -1){
                    // printf("%s: No such file or directory\n", arg[1]);
                    // fflush(stdout);
                    ;
                }
                else{
                    strcpy(OLDPWD, current_wd);
                }
            }
            continue;
        }
        else if(strcmp(table->vector[i].vector[0], "pwd") == 0){
            char wd[PWD_MAX_LENGTH];
            getcwd(wd, PWD_MAX_LENGTH);
            printf("%s\n", wd);
            continue;
        }
        
        pid = fork();
        if(pid == 0){
            execvp(table->vector[i].vector[0], table->vector[i].vector);
        }
    }
    
    dup2(savein, STDIN_FILENO);
    dup2(saveout, STDOUT_FILENO);
    close(savein);
    close(saveout);

    // waitpid(pid, NULL, 0);
    while(wait(NULL) > 0);
}

int main(){
    Tokens* tokens = NULL;
    Command_table* table = NULL;
    
    struct sigaction s;
    s.sa_handler = sigint_handler;
    sigemptyset(&s.sa_mask);
    s.sa_flags = SA_RESTART;
    sigaction(SIGINT, &s, NULL);

    do{
        if (sigsetjmp(env, 1) == 42) {
            printf("\n");
            // garbage_collect(tokens, table);
            // tokens = NULL;
            // table = NULL;
            continue;
        }

        tokens = read_line();
        // print_tokens(tokens);
        if(tokens->size == 0){
            garbage_collect(tokens, table);
            tokens = NULL;
            continue;
        }
        if(tokens->size > 0 && strcmp(tokens->vector[0], "exit") == 0){
            garbage_collect(tokens, table);
            tokens = NULL;
            printf("exit\n");
            break;
        }
        table = parse(tokens);
        // print_table(table);
        isexecuting = 1;
        execute(table);
        isexecuting = 0;

        garbage_collect(tokens, table);
        tokens = NULL;
        table = NULL;
    }while(true);
    exit(EXIT_SUCCESS);
}

/*
    BQ:
    BUG: if you call garbage_collect, the variables tokens & table is not set to NULL because pass by value
    Test case: 
    > 1.txt do nothine
    > 1.txt exit will only exit
    echo haha > 1.txt >> 2.txt
*/

/*
    For signal, 3 choices
    signal, longjmp, sigaction,
*/