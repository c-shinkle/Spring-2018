#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) 
{
  int a,c;
  pid_t pid = fork();
  a=1;
  c=2;
  if (pid==0) {
    printf("pid=%d, a=%d, c=%d\n",pid,a,c);
    a=10;
    c=20;
    wait(NULL);
    printf("pid=%d, a=%d, c=%d\n",pid,a,c);
  }
  printf("pid=%d, a=%d, c=%d\n",pid,a,c);
}
