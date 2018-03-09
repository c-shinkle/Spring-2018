#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_LINE 80

int checkForPipe(char *statement) {
  char tmp[MAX_LINE];
  strcpy(tmp, statement);
  char *parse = tmp;
  char *firstCmd = strtok_r(parse, "|", &parse);
  if (strcmp(strtok_r(NULL, "|", &parse), "more")==0)
    return 1;
  else {
    printf("\"more\" command required for pipe.\n");
    return 0;
  } 
}

int main(int argc, char **argv)
{
  printf("%d\n", checkForPipe("ls | a"));
}
