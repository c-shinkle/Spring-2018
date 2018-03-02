#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_LINE 80

typedef struct {
  int number;
  char command[MAX_LINE];
} cmd_t;

void removeLastChar(char *word) {
  word[strlen(word)-1] = 0;
}

int checkForAmpersand(int i, char **args) {
  if (strcmp(args[i-1], "&") == 0)
    return 1;
  return 0;
}

int checkForNewline (char *args) {
  if (*(args + strlen(args)-1) == '\n')
    return 1;
  return 0;
}
void parseInput(char *line, char **args, int *runConcurrently) {
  //char lineCopy[MAX_LINE];
  //strcpy(lineCopy, line);
  //printf("line: %s\n", line);
  //printf("lineCopy: %s\n", lineCopy);
  char *token = strtok(line, " ");
  int i;
  for (i=0;token && i < MAX_LINE/2;i++) {
    //printf("Token is %s", token);
    args[i] = token;
    token = strtok(NULL, " ");
  }
  
  //if (strcmp(args[i-1], "\n") == 0)
  //args[i-1] = NULL;
  
  if (checkForNewline(args[i-1] ) )
    removeLastChar(args[i-1] );
  
  *runConcurrently = checkForAmpersand(i, args);
  if (*runConcurrently == 1)
    args[i-1] = NULL;

  args[i] = NULL;
}

void printHistory(int i, cmd_t *cmds) {
  int j,k;
  if (i == -1)
    printf("No commands in history\n");
  else if (cmds[i].number>=10) {
    for (j=i,k=0;k<10;j--,k++) {
      if (j<0)
	j=9;
      printf("%d %s\n", cmds[j].number, cmds[j].command);
    }
  } else {
    for (j=i;j>=0;j--) 
      printf("%d %s\n", cmds[j].number, cmds[j].command);
  }
}

void updateHistory(int *i, int *counter, cmd_t *cmds, char *line) {
  strcpy(cmds[*i].command, line);
  cmds[*i].number = (*counter)++;
  *i = (*i+1)%10;
}

void runCommand(char **args, int runConcurrently) {
  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "Something went wrong with fork call\n");
  } else if (pid == 0) {
    //child process
    if (execvp(args[0],args) != 0) {
      fprintf(stderr, "Something went wrong while executing %s\n", args[0]);
      exit(1);
    }
    exit(0);
  } else {
    //parent
    if(!runConcurrently)
      wait(NULL);
  }
}

void printStartOfLine() {
  printf("osh> ");
  fflush(stdout);
}

int checkForHistory(char *word) {
  int x = 0;
  char c = word[0];
  if (c == '!') {
    c = word[1];
    if (48 <= c && c <= 57) {
      x = 1;
    }
  }
  return x;
}

void runHistory(int counter, int i, cmd_t *cmds, char **args) {
  if (counter == 1)
    printf("No commands have been entered yet.\n");
  else {
    char *cmdNum = args[0]+1;
    int num = atoi(cmdNum);
    int cur = (i==0) ? cmds[9].number : cmds[i-1].number;
    //printf("num=%d,cur=%d\n",num,cur);
    if (num > 0 && cur-9 <= num && num <= cur) {
      char tmp[MAX_LINE];
      int rc;
      strcpy(tmp, cmds[(num-1)%10].command);
      parseInput(tmp, args, &rc);
      runCommand(args, rc);
    } else {
      printf("No such command in history\n");
    }
  }
}

void runLastCommand(int counter, int i, cmd_t *cmds, char **args) {
  if (counter==1) {
    printf("No previous command\n");
    return;
  }
  char *tmp = (i==0) ? cmds[9].command : cmds[i-1].command;
  char lastCommand[MAX_LINE];
  strcpy(lastCommand, tmp);
  int rc;
  parseInput(lastCommand, args, &rc);
  runCommand(args, rc);
}

void changeDirectory(char ** args, char *prevDir) {
  if (args[1]) {
    if(*args[1]== '-') {
      if (prevDir[0]){
	if (chdir(prevDir))
	  fprintf(stderr, "System couldn't navigate to %s\n", args[1]);
      } else 
	printf("No previous directory\n");
    } else {
      if (*args[1] == '~') {
	char home[MAX_LINE];
	strcpy(home, getenv("HOME"));
	strcat(home, args[1]+1);
	args[1] = home;
      }
      if (chdir(args[1]))
	fprintf(stderr, "System couldn't navigate to %s\n", args[1]);
    }
    char curDir[MAX_LINE];
    if (getcwd(curDir, MAX_LINE) == NULL) 
      fprintf(stderr, "Something went wrong with getcwd...\n");
    else 
      strcpy(prevDir, curDir);
  } else {
    printf("No path given\n");
  }
}

void runPipedCommand(char **args, char *statementCopy) {
  char tmp[MAX_LINE];
  strcpy(tmp, statementCopy);
  char *parse = tmp;
  char *firstCmd = strtok_r(parse, "|", &parse);
  int rc;
  parseInput(firstCmd, args, &rc);
  
  int my_pipe[2];
  if(pipe(my_pipe) == -1) {
    fprintf(stderr, "Something went wrong with pipe.\n");
    return;
  }

  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "Something went wrong with fork call\n");
    return;
  } else if (pid == 0) {
    //child process
    close(my_pipe[0]);
    dup2(my_pipe[1], 1);
    if (execvp(args[0],args) != 0) {
      fprintf(stderr, "Something went wrong while executing %s\n", args[0]);
      exit(1);
    }
  } else {
    //parent process      
    wait(NULL);
    
    FILE *f = fopen(".output.txt", "w+");
    if (f == NULL) {
      printf("Something went wrong opening file\n");
      return;
    }
    close(my_pipe[1]);

    char reading_buf[1];
    while(read(my_pipe[0], reading_buf, 1) > 0) {
      fprintf(f, "%s", reading_buf);
    }
    close(my_pipe[0]);

    fclose(f);
    args[0] = "more";
    args[1] = ".output.txt";
    args[2] = NULL;
    runCommand(args, 0);
  }
  
}

int checkForPipe(char *statement) {
  char tmp[MAX_LINE];
  strcpy(tmp, statement);
  char *parse = tmp;
  char *firstCmd = strtok_r(parse, "|", &parse);
  if (strstr(strtok_r(NULL, "|", &parse), "more")!=0)
    return 1;
  else {
    printf("\"more\" command required for pipe.\n");
    return 0;
  } 
}

int main(void)
{
  char *args[MAX_LINE/2+1];
  char line[MAX_LINE], statementCopy[MAX_LINE], prevDir[MAX_LINE];
  char *statement, *line_ptr;
  cmd_t cmds[10];
  int should_run = 1, i = 0, counter = 1, runConcurrently;
  prevDir[0] = 0;
  while (should_run) {
    printStartOfLine();
    if((fgets(line, MAX_LINE, stdin) ) == NULL) {
      fprintf(stderr, "Something went wrong with fgets...\n");
    } else {
      runConcurrently = 0;
      line_ptr = line;
      statement = strtok_r(line_ptr, ";", &line_ptr);
      //while statement isn't null, do contents of loop, then get new token
      for (;statement;statement = strtok_r(NULL, ";", &line_ptr)) {
	strcpy(statementCopy, statement);
	if (strlen(statementCopy) && checkForNewline(statement))
	  removeLastChar(statementCopy);
	//printf("Before parse: statement %s args[0]%s\n", statement, args[0]);
	parseInput(statement, args, &runConcurrently);
	//printf("After parse: statement %s args[0]%s\n", statement, args[0]);
	//for (j = 0;args[j];j++)
	//printf("%s:length %d\n", args[j], strlen(args[j]));
	if (strcmp(args[0],"exit")==0) {
	  should_run = 0;
	} else {
	  if (strchr(statementCopy, '|') ) {
	    if(checkForPipe(statementCopy) )
	      runPipedCommand(args, statementCopy);
	  } else if (strcmp(args[0], "history")==0) {
	    printHistory(i-1, cmds);
	  } else if (strcmp(args[0], "!!")==0) {
	    runLastCommand(counter, i, cmds, args);
	  } else if (checkForHistory(args[0])) {
	    //printf("Before: statement %s flag %d\n", statement, runningHistoryCmd);
	    runHistory(counter, i, cmds, args);
	    //printf("After: statement %s flag %d\n", statement, runningHistoryCmd);
	  } else if (strcmp(args[0], "cd")==0) {
	    changeDirectory(args, prevDir);
	  } else {
	    runCommand(args, runConcurrently);
	  }
	  updateHistory(&i, &counter, cmds, statementCopy);
	}
      }
    }
  }
  wait(NULL);
  return 0;
} 
