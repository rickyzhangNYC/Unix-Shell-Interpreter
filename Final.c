#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

static pid_t pid;
static char *arguments[10];
static char *arguments2[10];
static char charBuffer [150];
static char* cmd;
static int bufferCounter = 0;
static int bufferCounter2 = 0;
static int argumentCounter = 0;
static int oldfdinput;
static int oldfdoutput;
static int pipeIndex;
static int status;

void fillcharBuffer(){
	arguments[bufferCounter] = strtok(charBuffer, " \n");
	bufferCounter = 0;
	bufferCounter2 = 0;
	while (arguments[bufferCounter] != NULL){
		bufferCounter ++;
		arguments[bufferCounter] = strtok(NULL, " \n");
	}
}

void runpipe();
void check();

int main(void){
	while (1){

		printf("Enter command: ");
		if (!fgets(charBuffer,150, stdin))
			return 0;

		fillcharBuffer();

		if (strcmp(arguments[0], "cd") == 0){
			if(arguments[1] == NULL)
				chdir("/");
			else
				chdir(arguments[1]);
		}
		pid = fork();

		switch(pid)
		{
		case -1: 
			return 1;
		case 0:
		{
			argumentCounter = 0;
			while (arguments[argumentCounter] != NULL){
				if (strcmp(arguments[argumentCounter], "<") == 0){
					if (arguments[argumentCounter+1] != NULL){
						oldfdinput = open(arguments[argumentCounter+1], O_RDONLY);
						dup2(oldfdinput, STDIN_FILENO);
					}
					else{
						printf("Error, input file not specified");
					}
					arguments[argumentCounter] = 0;
				}
				else if(strcmp(arguments[argumentCounter], ">") == 0){
					if (arguments[argumentCounter+1] != NULL){
						oldfdoutput = open(arguments[argumentCounter+1], O_CREAT, O_WRONLY, O_TRUNC, 0666);
						dup2(oldfdoutput, STDIN_FILENO);
					}
					else{
						printf("Error, output file not specified");
					}
					arguments[argumentCounter] = 0;
				}
				else if(strcmp(arguments[argumentCounter], "|") == 0){
					check();
					int fd[2];
					runpipe(fd);
				}
				else if(strcmp(arguments[argumentCounter], "&") == 0){
					arguments[argumentCounter] = NULL;
					fclose(stdin);
					execvp(arguments[0],arguments);
					exit(1);
				}
				argumentCounter++;
			}
			execvp(arguments[0], arguments);
			break;
		}
		default:
			waitpid(-1, &status, 0);
			break;
		}
	}
	return 0;
}

void check(){
	int i = 0;
	int j = 0;
	for (i = 0; arguments[i] != NULL; i++){
		if (!strcmp(arguments[i], "|")){
			arguments[i]= NULL;
			while(arguments[i+1] != NULL){
				arguments2[j] = arguments[i+1];
				arguments[i+1] = NULL;
				i++;
				j++;
			}
		}
	}
}

void runpipe(int pfd[]){
	if (pipe(pfd) < 0){
		perror("Error");
		exit(1);
	}
	switch (pid = fork()){
	case -1:
		perror("fork");
		exit(1);
	
	case 0:
		close(1);
		dup2(pfd[1],1);
		close(pfd[0]);
		execvp(arguments[0], arguments);
	default:
		close(0);
		dup2(pfd[0],0);
		close(pfd[1]);
		execvp(arguments2[0], arguments2); 
	 
	} 
}
