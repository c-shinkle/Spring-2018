#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>

void error_and_die(const char *msg) {
  printf("%s\n", msg);
  exit(EXIT_FAILURE);
}

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
  int inc_size = sizeof(int);
  int r;
  int *inc;
  const char *memname = "sample";
  const size_t region_size = sysconf(_SC_PAGE_SIZE);


  int fd = shm_open(memname, O_CREAT | O_TRUNC | O_RDWR, 0666);
  if (fd == -1)
    error_and_die("shm_open");
  
  r = ftruncate(fd, region_size);
  if (r != 0)
    error_and_die("ftruncate");
  
  void *ptr = mmap(0, region_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED)
    error_and_die("mmap");

  close(fd);
  
  pid_t pid = fork();
  if (pid != 0) { 
    //Parent process
    wait(NULL);
    for(inc = ptr; *inc != -1; inc += inc_size)
      printf("%d, ", *inc);
    printf("1\n");
    
    r = munmap(ptr, region_size);
    if (r != 0)
      error_and_die("munmap");
    r = shm_unlink(memname);
    if (r != 0)
      error_and_die("shm_unlink");

    return 0;
  } else {
    //Child process
    for(inc = ptr; n!=1; inc += inc_size) {
      *inc = n;
      if (n % 2 == 0) {
	n /= 2;
      } else {
	n = 3*n+1;
      }
    }
    *inc = -1;
  }
}
