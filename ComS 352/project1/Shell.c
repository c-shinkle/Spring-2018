#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_LINE 80
/**
 *cmd_t is used to store information for
 *the hisotry command
 */
typedef struct {
  int number;
  char command[MAX_LINE];
} cmd_t;
/**
 *This function removes the last character from word.
 */
void removeLastChar(char *word) {
  word[strlen(word)-1] = 0;
}
/**
 *This fucntion checks if an ampersand is in
 *the last element of args.
 */
int checkForAmpersand(int i, char **args) {
  if (strcmp(args[i-1], "&") == 0)
    return 1;
  return 0;
}
/**
 *This function checks if args ends
 *in a newline character.
 */
int checkForNewline (char *args) {
  if (*(args + strlen(args)-1) == '\n')
    return 1;
  return 0;
}
/**
 *This function parses line into tokens 
 *that are stored in args.
 *runConcurrently is changed to show whether or
 *not the command needs to concurrently or not.
 *Note: strtok will mutate line, so if the caller
 *doesn't want the date to be changed, they
 *need to make a copy in the calling scope.
 *I realize it would have been much more convientent to
 *make a copy in this function, but this caused errors
 *that I wasn't abel to debug. 
 */
void parseInput(char *line, char **args, int *runConcurrently) {
  char *token = strtok(line, " ");
  int i;
  for (i=0;token && i < MAX_LINE/2;i++) {
    args[i] = token;
    token = strtok(NULL, " ");
  }
  
  if (checkForNewline(args[i-1] ) )
    removeLastChar(args[i-1] );
  
  *runConcurrently = checkForAmpersand(i, args);
  if (*runConcurrently == 1)
    args[i-1] = NULL;

  args[i] = NULL;
}
/**
 *This function will iterate through the circular queue
 *cmds and will printf off its contents.
 */
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
/**
 *This function updates the history of the circular queue
 *at position i.
 */
void updateHistory(int *i, int *counter, cmd_t *cmds, char *line) {
  strcpy(cmds[*i].command, line);
  cmds[*i].number = (*counter)++;
  *i = (*i+1)%10;
}
/**
 *This function takes in the argurments for the command
 *to be ran and passes them into execvp(). There
 *is an optional wait() call if running concurrently
 *is desired.
 */
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
/**
 *This function flushes the stream
 */
void printStartOfLine() {
  printf("osh> ");
  fflush(stdout);
}
/**
 *This function checks to see if 
 *the formattin of word is correct
 *for the function for runHistory
 */
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
/**
 *This function uses the values of counter and i to find
 *the location in the circular queue of the command
 *from history to be ran. 
 */
void runHistory(int counter, int i, cmd_t *cmds, char **args) {
  if (counter == 1)
    printf("No commands have been entered yet.\n");
  else {
    char *cmdNum = args[0]+1;
    int num = atoi(cmdNum);
    int cur = (i==0) ? cmds[9].number : cmds[i-1].number;
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
/**
 *This function find the previous command using
 *counter and i and runs it.
 */
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
/**
 *This function changes the directory using the given
 *directory path in args. This functions also handles 
 *getting the previous directory using prevDir that
 *is stored in the main, and using ~ to find the home
 *directory. 
 */
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
/**
 *This function rans piped commands by getting the first instruction,
 *running it and piping its output to the parent process. Then,
 *the parent process stores its output in a hidden file called
 *.output.txt. Then, more() is called with .output.txt used as 
 *its parameter.  
 */
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
/**
 *This function checks if statement is well-formed
 *for runPipedCommand().
 */
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
/**
 *The main is documented in more detail in the readme.
 *This function declars all variables needed for the program.
 *It continuously parses input from the terminal and runs
 *the commands, mostly using runCommand. It also handles 
 *updating the circular queue cmds at the end of each iteration.
 */
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
      for (;statement;statement = strtok_r(NULL, ";", &line_ptr)) {
	strcpy(statementCopy, statement);
	if (strlen(statementCopy) && checkForNewline(statement))
	  removeLastChar(statementCopy);
	parseInput(statement, args, &runConcurrently);
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
	    runHistory(counter, i, cmds, args);
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
