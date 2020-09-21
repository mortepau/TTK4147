#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define N 5
#define LEFT phil
#define RIGHT ((phil + 1) % N)

pthread_mutex_t arbitrator;
pthread_mutex_t mtxs[N];
pthread_t philosophers[N];
int num[N] = {0, 1, 2, 3, 4};

static inline void nonOptimizedBusyWait(void){
    for(long i = 0; i < 10000000; i++){
        // "Memory clobber" - tells the compiler to optimize that all the memory
        // is being touched, and that therefore the loop cannot be optimized out
        asm volatile("" ::: "memory");
    }
}

void take_fork(int num){
    pthread_mutex_lock(&mtxs[num]);
}

void give_fork(int num){
    pthread_mutex_unlock(&mtxs[num]);
}


void* fn(void* args){

    int* j = args;
    int phil = *j;

    printf("Philosopher %d\n", phil);

    pthread_mutex_lock(&arbitrator);
    printf("Philosopher %d taking fork %d\n", phil, LEFT);
    take_fork(LEFT);
    printf("Philosopher %d took fork %d\n", phil, LEFT);
    nonOptimizedBusyWait();
    printf("Philosopher %d taking fork %d\n", phil, RIGHT);
    take_fork(RIGHT);
    printf("Philosopher %d took fork %d\n", phil, RIGHT);
    pthread_mutex_unlock(&arbitrator);

    // EATING
    printf("Philosopher %d eating\n", phil);
    nonOptimizedBusyWait();

    printf("Philosopher %d giving fork %d\n", phil, RIGHT);
    give_fork(RIGHT);
    printf("Philosopher %d gave fork %d\n", phil, RIGHT);
    nonOptimizedBusyWait();
    printf("Philosopher %d giving fork %d\n", phil, LEFT);
    give_fork(LEFT);
    printf("Philosopher %d gave fork %d\n", phil, LEFT);

    return NULL;
}

int main(){

    pthread_mutex_init(&arbitrator, NULL);

    for (int i = 0; i < N; i++){
        pthread_mutex_init(&mtxs[i], NULL);
    }
    
    for (int i = 0; i < N; i++){
        pthread_create(&philosophers[i], NULL, fn, &num[i]);
    }

    for (int i = 0; i < N; i++){
        pthread_join(philosophers[i], NULL);
    }

    for (int i = 0; i < N; i++){
        pthread_mutex_destroy(&mtxs[i]);
    }

    pthread_mutex_destroy(&arbitrator);
}