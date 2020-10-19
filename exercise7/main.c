#include "io.h"
#include <native/task.h>
#include <native/timer.h>
#include <sys/mman.h>
#include <rtdk.h>
#include <pthread.h>

#define A 1
#define B 2
#define C 3

#define NUM_DISTURBANCES 5

#define BILLION 1000*1000*1000
#define MILLION 1000*1000
#define PERIOD 1000*1000

struct taskArgs {
    int pin;
    long period;
};

int set_cpu(int cpu_number)
{
    cpu_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(cpu_number, &cpu);

    return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}

void busyTask(void* args){
    unsigned long endTime = rt_timer_read()+(unsigned long)60*BILLION;

    //int arg = ((struct taskArgs*)args)->pin;
    //long p = ((struct taskArgs*)args)->period;

	long arg = (long)args;
	//printf("struct: %p\npin: %d\n\rperiod: %lu\n\r", args, arg, p);
//	printf("pin: %lu\n\r", arg);
	
    while(1){
        if(io_read(arg) == 0){
            io_write(arg, 0);
            rt_timer_spin(5*MILLION);
            io_write(arg, 1);
        }

        if(rt_timer_read() > endTime){
            rt_printf("Time expired\n");
            rt_task_delete(NULL);
        }

        //if(rt_task_yield()){
        //    rt_printf("Task failed to yield\n");
        //    rt_task_delete(NULL);
        //}
		rt_task_wait_period(NULL);

    }
	rt_printf("Done in task\n");
}

void* disturb(void* args)
{
    set_cpu(1);

    while(1)
    {
        asm volatile("" ::: "memory");
    }
}

int main(){

    mlockall(MCL_CURRENT|MCL_FUTURE);

    rt_print_auto_init(1);
    io_init();

    RT_TASK busyTasks[3];
	pthread_t disturbance[NUM_DISTURBANCES];

    for(int i = 0; i < 3; i++){
        rt_task_create(&busyTasks[i], (const char*)(i+"A"-1), 0, 1, T_CPU(1)); 
    }

	//for(int i = 0; i < NUM_DISTURBANCES; i++){
	//	pthread_create(&disturbance[i], NULL, disturb, NULL);
	//}

	for(int i = 0; i < 3; i++){
		rt_task_set_periodic(&busyTasks[i], TM_NOW, PERIOD);
	}

    for(long i = 0; i < 3; i++){
        rt_task_start(&busyTasks[i], &busyTask, (void*)(i+1));//(&(struct taskArgs){i+1, 1000}));
    }

	//for (int i = 0; i < NUM_DISTURBANCES; i++) {
    //    pthread_join(disturbance[i], NULL);
    //}
	while(1);
    return 0;
}