#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
//#include <unistd.h>
#define MAX_LINE 80

char *trim(char *str) {
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;
  
  return str;
}

int  parseInput(char *line, char **args) {
  char *token = strtok(line, " ");
  int i;
  for (i=0;token && i < MAX_LINE/2;i++) {
    args[i] = token;
    token = strtok(NULL, " ");
  }
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
    for (i = 0; args[i]; i++)
      printf("%s\n",args[i]);
    
  }
  return 0;
}
