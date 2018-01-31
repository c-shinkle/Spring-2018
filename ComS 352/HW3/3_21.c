#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("This program needs commandline an arguments\n");
		return 1;
	}
	pid_t pid = fork();
	if (pid != 0) { //Parent process
		wait(NULL);
		
	} else {//Child process
		
	}
	
	return 0;
}