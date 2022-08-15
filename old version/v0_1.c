#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h> // safer goto
#include <setjmp.h> // similar to goto
//#include "gc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "main.h"

#define MAX_INPUT_LENGTH 1024 // 可能还得加
#define MAX_PWD_LENGTH 100 // pwd用

#define MAX_ARG_SIZE 50
#define MAX_ARG_NUMBER 50
#define MAX_PIPELINE 40
#define MAX_JOB 20

#define HARD 2
#define EASY 1

pid_t mainpid;

void add_blank(char* line, int from, int hard){// c居然没有bool 233
// same for > < | and >> TAT
	int i;
	// left blank ' '
	for (i = strlen(line); i >= from; i--){
		line[i+1] = line[i];
	}
	line[from] = ' ';
	// hard mode
	if(hard == HARD){		
		// right blank ' '
		for (i = strlen(line); i >= from + 3; i--){
			line[i+1] = line[i];
		} 
		line[from+3] = ' ';
	}
	// easy mode
	else{
		// right blank ' '
		for (i = strlen(line); i >= from + 2; i--){
			line[i+1] = line[i];
		} 
		line[from+2] = ' ';
	}
}

void shift(char** arg, int probe){
	int i;
	for(i = probe; i <= MAX_ARG_NUMBER - 3; i++){
		arg[i] = arg[i+2];
	}
	arg[MAX_ARG_NUMBER -2] = NULL;
	arg[MAX_ARG_NUMBER -1] = NULL;
}

void execute_single(char** arg, int k){
	if(k == 0){
	}
//  --------------------- haddle > >> < -----------------------
	int probe = 0;
	//int already = 0;
	while(arg[probe] != NULL){
		//printf("%d\n", redcom[k][probe]);
		if (strcmp(arg[probe], "<") == 0){// && redcom[k][probe+2*already] == 1){
			//duplicate_input ++;
			//already++;
			int fd;
			fd = open(arg[probe+1], O_RDONLY, 0644);
			if(fd == -1){
				printf("%s: No such file or directory\n", arg[probe+1]);
				fflush(stdout);
				exit(EXIT_FAILURE);
			}
			dup2(fd, STDIN_FILENO);
			close(fd);
			shift(arg, probe);
			probe--;
		}
		else if (strcmp(arg[probe], ">") == 0){// && redcom[k][probe+2*already] == 1){
			//duplicate_output ++;
			//already++;
			int fd;
			fd = open(arg[probe+1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
			if(fd == -1){
				printf("%s: Permission denied\n", arg[probe+1]);
				fflush(stdout);
				exit(EXIT_FAILURE);
			}
			dup2(fd, STDOUT_FILENO);
			close(fd);
			shift(arg, probe);
			probe--;
		}
		else if (strcmp(arg[probe], ">>") == 0){// && redcom[k][probe+2*already] == 1){
			//duplicate_output ++;
			//already++;
			int fd;
			fd = open(arg[probe+1], O_WRONLY|O_CREAT|O_APPEND, 0644);
			if(fd == -1){
				printf("%s: Permission denied\n", arg[probe+1]);
				fflush(stdout);
				exit(EXIT_FAILURE);
			}
			dup2(fd, STDOUT_FILENO);
			close(fd);
			shift(arg, probe);
			probe--;
		}
		probe++;
	}

// ---------  built in pwd --------------
    if (strcmp(arg[0], "pwd") == 0){
		char wd[MAX_PWD_LENGTH];
		getcwd(wd, MAX_PWD_LENGTH);
		printf("%s\n", wd);
		fflush(stdout);
		return;
	}

//  ------------------ call execvp function ------------------
	if (execvp(arg[0], arg) < 0){
			printf("%s: command not found\n", arg[0]);
			fflush(stdout);
			exit(EXIT_FAILURE);
	}
	return;
}

void execute_recursion(char*** command, int k, int i){
	if(k > i){
		return;
	}
	// kth command out of 0~i
	pid_t pid;
	int fd[2];
	pipe(fd);
	int status;

	pid = fork();
	if(pid == -1){
		perror("error at fork");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0){
		close(fd[0]);
		if(k != i){
			dup2(fd[1], STDOUT_FILENO);
			close(fd[1]);	
		}
		execute_single(command[k], k);
		exit(EXIT_SUCCESS);
	}
	else{
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]); 
		execute_recursion(command, k + 1, i);
		waitpid(pid, &status, 0);
	}
	return;
}

char *append(char *s, char c) {
    int len = strlen(s);
    char buf[len+2];
    strcpy(buf, s);
    buf[len] = c;
    buf[len + 1] = 0;
    return strdup(buf);
}

int main(){

// alllocate memory
	
	int red[MAX_ARG_NUMBER];
	int redcom[MAX_PIPELINE][MAX_ARG_NUMBER];

	char* line = (char*)calloc(MAX_INPUT_LENGTH, sizeof(char));

	char** arg = (char**)calloc(MAX_ARG_NUMBER, sizeof(char*));
	for(int z = 1; z < MAX_ARG_NUMBER; z++){
		arg[z] = (char*)calloc(MAX_ARG_SIZE, sizeof(char));
	}

	char*** command = (char***)calloc(MAX_PIPELINE, sizeof(char**) );
	for(int z = 0; z < MAX_PIPELINE; z++){
		command[z] = (char**)calloc(MAX_ARG_NUMBER, sizeof(char*));
		for(int y = 0; y < MAX_ARG_NUMBER; y++){
			command[z][y] = (char*)calloc(MAX_ARG_SIZE, sizeof(char));
		}
	}

	int saveinn = dup(STDIN_FILENO);
	int saveoutt = dup(STDOUT_FILENO);

	signal(SIGINT, myHandler);
	if(sigsetjmp(env, 1) == 2){
	 	dup2(saveinn, STDIN_FILENO);
	 	dup2(saveoutt, STDOUT_FILENO);
	}

//
	while(1){

	// initialize the variables
		// line = "asss";

		for(int z = 0; z < MAX_ARG_NUMBER; z++){
			arg[z] = "";
			red[z] = 0;
		}

		for(int z = 0; z < MAX_PIPELINE; z++){
			for(int y = 0; y < MAX_ARG_NUMBER; y++){
				command[z][y] = "";
				redcom[z][y] = 0;
			}
		}


	// prompt
	if(background == 0){
		fflush(stdin);
		printf("mumsh $ ");
		fflush(stdout);
	}
	else{
		background = 0;
	}
	// * * * * * * * * * * read line * * * * * * * * * * * * * * *

		int charcount = 0;
		char ch;
		int quote_read = 00;
		int last_red = -1;
		int still_need_red = 1;

		int red_app = 0;
		int blank_count = 0;

		while(1){		
			ch = (char)fgetc(stdin);

			if(ch == '\n' && charcount == 0){
				siglongjmp(env, 2);
			}

			// syntax errir case 20			
			if(quote_read == 0){
				if((ch == '>' || ch == '<' || ch == '|') && blank_count > 0 && red_app == 1){
					red_app = 0;
					blank_count = 0;
					printf("syntax error near unexpected token `%c'\n", ch);
					fflush(stdout);
					while((char)fgetc(stdin) != '\n'){
						continue;
					}
					siglongjmp(env, 2);
				}
				else if(ch == '>' || ch == '<'){
					red_app = 1;
				}
				else if(red_app == 1 && ch == ' '){
					blank_count ++;
				}
				else if(ch != ' ' || ch != '>' || ch != '<' || ch != '|'){
					blank_count = 0;
					red_app = 0;
				}
				//printf("this char lead red_app %d blank_count %d \n", red_app, blank_count);
			}
			//printf("should not be here\n");

			if(ch == '\'' ){
				if (quote_read == 00){
					quote_read = 10;
					line[charcount] = ch;
					charcount++;
				}
				else if(quote_read == 10){
					quote_read = 00;
					line[charcount] = ch;
					charcount++;
				}
				else if(quote_read == 01){
					line[charcount] = ch;
					charcount++;
				}
			}
			else if(ch == '"' ){
				if (quote_read == 00){
					quote_read = 01;
					line[charcount] = ch;
					charcount++;
				}
				else if(quote_read == 01){
					quote_read = 00;
					line[charcount] = ch;
					charcount++;
				}
				else if(quote_read == 10){
					line[charcount] = ch;
					charcount++;
				}
			}
			else if(ch == '\n' ){
				still_need_red = 1;
				for(int zz = last_red + 1; zz < charcount; zz++){
					if(line[zz] != ' '){
						still_need_red = 0;
					}
				}
				//printf("hh  %d\n", still_need_red);
				if(quote_read == 0){
					if(still_need_red == 0){
						line[charcount] = '\0';
// background identify
						if(line[charcount - 1] == '&'){
							line[charcount - 1] = ' ';
							background = 1;
							joblist[jobcount].history = (char*)calloc(MAX_INPUT_LENGTH, sizeof(char));
							//printf("%s\n-------\n", line);
							strncpy(joblist[jobcount].history, line, charcount - 1);
							
							jobcount++;
							printf("[%d] %s&\n",jobcount,joblist[jobcount-1].history);
							fflush(stdout);
						}
						break;
					}
					else{
						printf("> ");
						fflush(stdout);
						continue;
					}
				}
				else{
					line[charcount] = ch;
					charcount++;
					printf("> ");
					fflush(stdout);
				}	
			}
			else if(ch == EOF){
				if(charcount == 0){
				// ctrl + D
					printf("exit\n");
					fflush(stdout);
					//clean_up(line, arg, command);
					exit(EXIT_SUCCESS);
				}
				else{
					line[charcount] = '\0';
					break;
				}
			}
			else{
				if(quote_read == 00){
					if(ch == '>' || ch == '<' || ch == '|'){
						last_red = charcount;
					}
				}
				line[charcount] = ch;
				charcount++;
			}

			//printf("%d ---- %d\n", quote_read, still_need_red);
		}
		//line[strcspn(line, "\n")] = '\0';
		if(strncmp(line, "exit", 4) == 0){
			printf("exit\n");
			fflush(stdout);
			//clean_up(line, arg, command);
			exit(EXIT_SUCCESS);
		}

		if (strncmp(line, "jobs", 4) == 0){
			if(jobcount > 0){
				for(int i = 0; i < (jobcount+1)/2; i++)
					joblist[i].done = 1;
			}

			for(int z = 0; z < jobcount; z++){
				printf("[%d] ", z + 1);
				fflush(stdout);
				if (joblist[z].done == 0)
					printf("running %s&\n", joblist[z].history);
				else if(joblist[z].done == 1)
					printf("done %s&\n", joblist[z].history);
				fflush(stdout);
			}
			continue;
		}

	// * * * * * * * * * * parse * * * * * * * * * * * * * * * 
		// fuck, we need to deal with quotation 
		int within_quote = 0;// 10 for single, 01 for double
		int ith = 00; // to put in the ith arg
		char cha;

		for(int z = 0; z < MAX_INPUT_LENGTH; z++){
			//printf("ith is %d\n", ith);
			cha = line[z];
			if (cha == '\0')
				break;

			// no quote case
			if(within_quote == 00){
				if(cha == ' '){
				// tok tieh blank and multiple blanks
					if(z == 0){
					}
					else if(z > 0 && line[z - 1] == ' '){
						//continue;
					}
					else{
						ith ++;
						//continue;
					}	
				}
				else if(cha == '\''){
					within_quote = 10;
					continue;
				}
				else if(cha == '"'){
					within_quote = 01;
					continue;
				}
				else if(cha == '<' || cha == '|'){
					red[ith] = 1;
					if(strcmp(arg[ith], "") == 0){
						arg[ith] = append(arg[ith], cha);
					}
					else{
						ith++;
						arg[ith] = append(arg[ith], cha);
					}
					if(line[z+1] != ' ')
						ith++;
				}
				else if(cha == '>'){
					red[ith] = 1;
					if(line[z + 1] == '>'){
						if(strcmp(arg[ith], "") == 0){
							arg[ith] = append(arg[ith], cha);
							arg[ith] = append(arg[ith], cha);
						}
						else{
							ith++;
							arg[ith] = append(arg[ith], cha);
							arg[ith] = append(arg[ith], cha);
						}
						if(line[z+2] != ' ')
							ith++;
						z++;
					}
					else{
						if(strcmp(arg[ith], "") == 0){
							arg[ith] = append(arg[ith], cha);
						}
						else{
							ith++;
							arg[ith] = append(arg[ith], cha);
						}
						if(line[z+1] != ' ')
							ith++;
					}
				}
				else{// normal char
					// fucking bus error for strncat
						arg[ith] = append(arg[ith], cha);
				}
			}
			// single quote case
			else if(within_quote == 10){
				if(cha == '\''){
					within_quote = 00;
					continue;
				}
				else{
					arg[ith] = append(arg[ith], cha);
				}
			}
			// double quote case
			else{
				if(cha == '"'){
					within_quote = 00;
					continue;
				}
				else{
					arg[ith] = append(arg[ith], cha);
				}
			}
			//printf("%d ---  %s\n", ith, arg[ith]);
		}
		if(strcmp(arg[ith], "") == 0){
			arg[ith] = NULL;
		}
		else{
			arg[ith + 1] = NULL;
		}

		/*
		//  ------------------ haddle > >> < and | ------------------
		char *probe;
		probe = strchr(line, '|');
		//printf("we are here");
		while(probe != NULL){
			add_blank(line, probe - line, EASY); // used many times
			probe = strchr(probe + 2, '|');// cope with |
			//printf("once");
		}
		probe = strchr(line, '<');
		//printf("we are here");
		while(probe != NULL){
			add_blank(line, probe - line, EASY); // used many times
			probe = strchr(probe + 2, '<');// cope with <
		}
		//	both > and >>
		probe = strchr(line, '>');
		//printf("we are here");
		while(probe != NULL){
			if(line[probe-line+1] != '>'){
				add_blank(line, probe - line, EASY); // used many times
				probe = strchr(probe + 2, '>');// cope with >
			}
			else{
				add_blank(line, probe - line, HARD); // used many times
				probe = strchr(probe + 3, '>');// cope with >>
			}
		//printf("once");
	 	}
		//  -------- finally use strtok to handle seperation ---------
		int wordcount = 0;
		arg[0] = strtok(line, " ");
		while(arg[wordcount] != NULL){
			wordcount++;
			arg[wordcount] = strtok(NULL, " ");
		}
		arg[wordcount] = NULL;
		*/
		
	// * * * * * * * * * * execute * * * * * * * * * * * * * * *

		// handle enter so that we do not get segmentation error
		if(arg[0] == NULL){
			continue;
		}

		//  ----------------- built-in command cd --------------------
		if (strcmp(arg[0], "cd") == 0){
			if(arg[1] == NULL || strcmp(arg[1], "~") == 0){
				chdir(getenv("HOME"));
			}
			else if(strcmp(arg[1], ".") == 0){
				;
			}
			//else if(arg[1] == ".."){
			//	;
			//}
			else{
				int cderror;
				cderror = chdir(arg[1]);
				if(cderror == -1){
					printf("%s: No such file or directory\n", arg[1]);
					fflush(stdout);
				}
			}	
			continue;
		}

		// ----------divide arg into several commands with tok | ----------
		int flag = 0;
		int i = 0; // how many commands
		int j = 0;

		while(arg[flag] != NULL){
			if(strcmp(arg[flag], "|") == 0 && red[flag] == 1){
				command[i][j] = NULL;
				j = 0;	
				i++;
				flag++;
				continue;
			}
			command[i][j] = arg[flag];
			redcom[i][j] = red[flag];
			j++;
			flag++;
		}
		command[i][j] = NULL;
		// the trick of char*** is unbelievable

		if(i > 0){
			for(int z = 0; z < i; z++){
				//if(strcmp(command[z][0], "") == 0){
				if(command[z][0] == NULL){
					printf("error: missing program\n");
					fflush(stdout);
					break;
				} 
			}
		}
		
	// input output duplicate error
		int input_dup = 0;
		int output_dup = 0;
		for(int z = 0; z < i + 1; z ++){
			int probe = 0;
			int duplicate_input = 0;
			int duplicate_output = 0;

			while(command[z][probe] != NULL){
				if (strcmp(command[z][probe], "<") == 0 && redcom[z][probe] == 1){
					duplicate_input ++;
				}
				else if (strcmp(command[z][probe], ">") == 0 && redcom[z][probe] == 1){
					duplicate_output ++;
				}
				else if(strcmp(command[z][probe], ">>") == 0 && redcom[z][probe] == 1){
					duplicate_output ++;
				}
				probe++;
			}

			if(i == 0){
				;
			}
			else{
				if(z == 0){
					duplicate_output ++;
				}
				else if(z == i){
					duplicate_input ++;
				}
				else{
					duplicate_input ++;
					duplicate_output ++;
				}
			}

			if(duplicate_input > 1){
				input_dup = 1;
			}
			if(duplicate_output > 1){
				output_dup = 1;
			}
		}

		if(input_dup == 1){
			printf("error: duplicated input redirection\n");
			fflush(stdout);
			continue;
		}
		if(output_dup == 1){
			printf("error: duplicated output redirection\n");
			fflush(stdout);
			continue;
		}

		if(background == 1){
			printf("mumsh $ ");
			fflush(stdout);
		}

		// the stdin and stdout is missing
		int savein = dup(STDIN_FILENO);
		int saveout = dup(STDOUT_FILENO);
		execute_recursion(command, 0, i);
		dup2(savein, STDIN_FILENO);
		dup2(saveout, STDOUT_FILENO);

	}

	return 0;
}













