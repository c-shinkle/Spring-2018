#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include <unistd.h>
#define MAX_LINE 80

void removeNewlineChar(char *word) {
  word[strlen(word)-1] = 0;
}

int parseInput(char *line, char **args) {
  char *token = strtok(line, " ");
  int i;
  for (i=0;token && i < MAX_LINE/2;i++) {
    args[i] = token;
    token = strtok(NULL, " ");
  }
  removeNewlineChar(args[i-1]);
  args[i] = NULL;
  return 0;
}

int main(void)
{
  char *args[MAX_LINE/2+1];
  char line[MAX_LINE];
  int should_run = 1, i, err;
  while (should_run) {
    printf("osh> ");
    fflush(stdout);
    //1. fork child process
    //2. child process will invoke execvp()
    //3. if command included &, parent will invoke wait()
    
    if ( (fgets(line, MAX_LINE, stdin) ) == NULL) {
      fprintf(stderr, "Something went wrong with fgets...\n");
      should_run = 0;
    }
    
    err = parseInput(line, args);
    if (err) {
      fprintf(stderr, "Something went wrong with parsing input...\n");
      should_run = 0;
    }
    //printf("%s %d\n",args[0],strcmp(args[0],"exit"));
    if (strcmp(args[0],"exit")==0) {
      should_run = 0;
    } else {
      pid_t pid;
      pid = fork();
      if (pid < 0) {
	fprintf(stderr, "Something went wrong with fork call\n");
      } else if (pid == 0) {
	//child process
	if (execvp(args[0],args) < 0) {
	  fprintf(stderr, "Something went wrong with execvp\n");
	  return 1;
	}
      } else {
	//parent
	wait(NULL);
      }
    }
  }
  return 0;
}
