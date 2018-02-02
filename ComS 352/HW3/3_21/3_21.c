#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
  if (argc == 1) {
    printf("This program needs command-line arguments.\n");
  }

  int n = atoi(argv[1]);

  if (n == 0) {
    printf("A positive integer is required.\n");
    return 1;
  }
  
  pid_t pid = fork();
  if (pid != 0) { 
    //Parent process
    wait(NULL);
    printf("1\n");
    return 0;
  } else {
    //Child process
    while(n!=1) {
      printf("%d, ",n);
      if (n % 2 == 0) {
	n /= 2;
      } else {
	n = 3*n+1;
      }
    }
  }
}
