#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include <unistd.h>
#define MAX_LINE 80

typedef struct {
  int number;
  char command[MAX_LINE];
} cmd_t;

void removeNewlineChar(char *word) {
  word[strlen(word)-1] = 0;
}

void parseInput(char *line, char **args) {
  char *token = strtok(line, " ");
  int i;
  for (i=0;token && i < MAX_LINE/2;i++) {
    args[i] = token;
    token = strtok(NULL, " ");
  }
  removeNewlineChar(args[i-1]);
  args[i] = NULL;
}

void printHistory(int i, cmd_t *cmds) {
  int j,k;
  if (cmds[i].number>=10) {
    for (j=i,k=0;k<10;j--,k++) {
      if (j<0)
	j=9;
      printf("%d %s\n", cmds[j].number, cmds[j].command);
    }
  } else {
    for (j=0;j<cmds[i].number;j++) 
      printf("%d %s\n", cmds[j].number, cmds[j].command);
  }
}

int main(void)
{
  char *args[MAX_LINE/2+1];
  char line[MAX_LINE];
  cmd_t cmds[10];
  int should_run = 1, i = 0, counter = 1;
  while (should_run) {
    printf("osh> ");
    fflush(stdout);
    
    if ( (fgets(line, MAX_LINE, stdin) ) == NULL) {
      fprintf(stderr, "Something went wrong with fgets...\n");
      should_run = 0;
    }
    char tmp[MAX_LINE];
    strcpy(tmp, line);
    parseInput(tmp, args);

    //printf("%s %d\n",args[0],strcmp(args[0],"exit"));
    if (strcmp(args[0],"exit")==0) {
      should_run = 0;
    } else if (strcmp(args[0], "!!")==0) {
      printHistory(i-1, cmds);
    } else {
      strcpy(cmds[i].command, line);
      cmds[i].number = counter++;
      i = (i+1)%10;
      /*
      if (execvp(args[0],args) != 0) {
	fprintf(stderr, "Something went wrong with execvp\n");
	return 1;
      }
      */
      pid_t pid;
      pid = fork();
      if (pid < 0) {
	fprintf(stderr, "Something went wrong with fork call\n");
      } else if (pid == 0) {
	//child process
	if (execvp(args[0],args) != 0) {
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
