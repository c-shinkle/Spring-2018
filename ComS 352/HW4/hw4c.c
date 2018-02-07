#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>

#define BUFFER_SIZE 9
#define READ_END 0 
#define WRITE_END 1

void reverseCase(const char *src, char *dest, int len) {
  int i;
  for(i=0; i < len; i++) {
      if(src[i] <= 'Z' && src[i] >= 'A') {
	  *(dest + i) = (char)tolower((int)src[i]);
	}
      else if(src[i] <= 'z' && src[i] >= 'a') {
	  *(dest + i) = (char)toupper((int)src[i]);
	}
      else {
	  *(dest + i) = src[i];
	}
    }

  i++;
  *(dest + i) = '\0';
}

int main(void) 
{ 
  char write_msg[BUFFER_SIZE] = "Hi There"; 
  char read_msg[BUFFER_SIZE]; 
  int fd[2]; 
  pid_t pid;

  /* create the pipe */ 
  if (pipe(fd) == -1) {
    fprintf(stderr,"Pipe failed");
    return 1;
  } 
  /* fork a child process */ 
  pid = fork();
  
  if (pid < 0) { 
    /* error occurred */ 
    fprintf(stderr, "Fork Failed");
    return 1;
  }
  if (pid > 0) { 
    /* parent process */ /* close the unused end of the pipe */ 
    close(fd[READ_END]);
    
    /* write to the pipe */ 
    write(fd[WRITE_END], write_msg, BUFFER_SIZE);
    
    /* close the write end of the pipe */ 
    close(fd[WRITE_END]);

  } else { 
    /* child process */ 
    /* close the unused end of the pipe */ 
    close(fd[WRITE_END]);

    /* read from the pipe */ 
    read(fd[READ_END], read_msg, BUFFER_SIZE);
    
    char final_msg[BUFFER_SIZE];
    
    reverseCase(read_msg, final_msg, BUFFER_SIZE);

    printf("read %s\n", final_msg);
    
    /* close the write end of the pipe */ 
    close(fd[READ_END]);
  } 
  return 0;
}
