#include <stdio.h>
#include <pthread.h>

extern void *world();
extern void *hello();
extern void *exclamation();

int main(int argc, char *argv[]) {
  pthread_t t0, t1, t2;
  int semaphore = 0;
  pthread_create(&t0, NULL, hello, &semaphore);
  pthread_create(&t1, NULL, world, &semaphore);
  pthread_create(&t2, NULL, exclamation, &semaphore);
  
  pthread_join(t0, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  printf("\n");
  return(0);
}

void *hello(int *s) {
  printf("hello ");
  (*s)++;
  return NULL;
}

void *world(int *s) {
  while(*s != 1)
    ;
  printf("world");
  (*s)++;
  return NULL;
}

void *exclamation(int *s) {
  while(*s != 2)
    ;
  printf("!");
  return NULL;
}
