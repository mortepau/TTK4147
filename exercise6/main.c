#define _GNU_SOURCE

#include "io.h"
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <sched.h>

#define TA_A 1
#define TA_B 2
#define TA_C 3

#define NUM_DISTURBANCES 5

#define BILLION 1000*1000*1000

struct taskArgs {
    int pin;
    long period;
};

void busy_wait_ms(int delay)
{
    clock_t start = clock();

    while (clock() < start + delay) {}
}

struct timespec timespec_normalized(time_t sec, long nsec)
{
    while (nsec >= BILLION) {
        nsec -= BILLION;
        sec++;
    }
    while (nsec <0) {
        nsec += BILLION;
        sec--;
    }
    return (struct timespec){sec, nsec};
}

struct timespec timespec_add(struct timespec lhs, struct timespec rhs)
{
    return timespec_normalized(lhs.tv_sec + rhs.tv_sec, lhs.tv_nsec + rhs.tv_nsec);
}

int set_cpu(int cpu_number)
{
    cpu_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(cpu_number, &cpu);

    return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}

void* fn(void* args)
{
    set_cpu(1);
    int arg = ((struct taskArgs*)args)->pin;
    long p = ((struct taskArgs*)args)->period;
    
    struct timespec waketime;
    clock_gettime(CLOCK_REALTIME, &waketime);

    struct timespec period = {.tv_sec = 0, .tv_nsec = p};

    while (1)
    {
        if (io_read(arg) == 0)
        {
            io_write(arg, 0);
            busy_wait_ms(5);
            io_write(arg, 1);
        }

        waketime = timespec_add(waketime, period);
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &waketime, NULL);
    }

    return NULL;
}

void* disturb(void* args)
{
    set_cpu(1);

    while(1)
    {
        asm volatile("" ::: "memory");
    }
}

int main(int argc, char* argv[])
{
    pthread_t p_A, p_B, p_C;
    pthread_t disturbance[NUM_DISTURBANCES];

    //printf("Initializing comedi\n");
    io_init();
    //printf("Initializing comedi done\n");

    //printf("Creating threads\n");
    pthread_create(&p_A, NULL, fn, (&(struct taskArgs){TA_A, 1*1000*1000}));
    pthread_create(&p_B, NULL, fn, (&(struct taskArgs){TA_B, 1*1000*1000}));
    pthread_create(&p_C, NULL, fn, (&(struct taskArgs){TA_C, 1*1000*1000}));

    for (int i = 0; i < NUM_DISTURBANCES; i++) {
        pthread_create(&disturbance[i], NULL, disturb, NULL);
    }

    //printf("Joining threads\n");
    pthread_join(p_A, NULL);
    pthread_join(p_B, NULL);
    pthread_join(p_C, NULL);

    for (int i = 0; i < NUM_DISTURBANCES; i++) {
        pthread_join(disturbance[i], NULL);
    }

    return 0;
}