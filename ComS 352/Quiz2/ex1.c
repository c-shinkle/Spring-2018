#include <stdio.h>
#include <pthread.h>
#define NUM_THREADS 5

void *PrintHello(void *threadid)
{
  printf("\n%d: Hello World from Thread #%d !\n",
	 threadid, pthread_self());
  pthread_exit(NULL);
}


int main()
{
  pthread_t threads[NUM_THREADS];
  int rc, t;
  for (t=0;t < NUM_THREADS;t++) {
    printf("Creating thread %d\n", t);
    rc = pthread_create(&threads[t], NULL, PrintHello, (void *) t);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      return -1;
    }
  }

  for(t=0; t<NUM_THREADS;t++) 
    pthread_join(threads[t],NULL);
  pthread_exit(NULL);
  return 0;
}
