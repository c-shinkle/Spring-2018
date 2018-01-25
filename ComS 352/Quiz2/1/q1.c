#include <stdio.h>
#include <pthread.h>

void *PrintHello(pthread_t pt) {
  printf("Hello\n");
  pthread_join(pt, NULL);
}

void *PrintWorld() {
  printf("World!\n");
  pthread_exit(NULL);
}


int main() 
{
  pthread_t thread1, thread2;
  int a = 0;
  int b = 1;
  int rc;

  rc = pthread_create(&thread1, NULL, PrintHello(thread2), (void *) a);
  if (rc) {
    printf("Something went wrong with thread%d with error %d\n", a, rc);
    return -1;
  }
 
  rc = pthread_create(&thread2, NULL, PrintWorld, (void *) b);
  if (rc) {
    printf("Something went wrong with thread%d with error %d\n", b, rc);
    return -1;
  }

  //pthread_join(thread1, NULL);
  //pthread_join(thread2, NULL);
  pthread_exit(NULL);

  return 0;
}
