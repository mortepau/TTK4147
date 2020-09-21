#include <stdio.h>
#include <pthread.h>
// #include <semaphore.h>

#define MILLION 1000000

long g = 0;
// sem_t sem;

void* fn(void* args){
    long l = 0;

    for(long i = 0; i < 50 * MILLION; i++){
        l++;
        // sem_wait(&sem);
        g++;
        // sem_post(&sem);
    }

    printf("Local variable: %lu\nGlobal variable: %lu\n", l, g);

    return NULL;
}

int main(){
    // sem_init(&sem, 0, 1);
    pthread_t inc1, inc2;

    pthread_create(&inc1, NULL, fn, NULL);
    pthread_create(&inc2, NULL, fn, NULL);

    pthread_join(inc1, NULL);
    pthread_join(inc2, NULL);
}