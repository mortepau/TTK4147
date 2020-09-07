#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <string.h>
#include <x86intrin.h>
#include <sched.h>

#define BILLION 1000000000
#define FREQUENCY 2660000000
#define FREQUENCY_NS 2.66

struct timespec timespec_normalized(time_t sec, long nsec) {
    while (nsec >= BILLION) {
        nsec -= BILLION;
        ++sec;
    }
    while (nsec < 0) {
        nsec += BILLION;
        --sec;
    }
    return (struct timespec){sec, nsec};
}

struct timespec timespec_sub(struct timespec lhs, struct timespec rhs) {
    return timespec_normalized(lhs.tv_sec - rhs.tv_sec, lhs.tv_nsec - rhs.tv_nsec);
}

struct timespec timespec_add(struct timespec lhs, struct timespec rhs) {
    return timespec_normalized(lhs.tv_sec + rhs.tv_sec, lhs.tv_nsec + rhs.tv_nsec);
}

int timespec_cmp(struct timespec lhs, struct timespec rhs) {
    if (lhs.tv_sec < rhs.tv_sec) {
        return -1;
    }
    if (lhs.tv_sec > rhs.tv_sec) {
        return 1;
    }
    return lhs.tv_nsec - rhs.tv_nsec;
}

double timespec_to_ticks(struct timespec t, double freq){
    int duration = t.tv_sec*BILLION+t.tv_nsec;
    return (double)((double)duration*freq);
}

void busy_wait(struct timespec t) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    struct timespec then = timespec_add(now, t);

    while (timespec_cmp(now, then) < 0) {
        for (int i = 0; i < 10000; i++) {
            clock_gettime(CLOCK_MONOTONIC, &now);
        }
    }
}

void busy_times(struct timespec t) {
    struct tms now;
    times(&now);
    //double tt = (double)now + timespec_to_ticks(t, freq);
    //printf("%f, %f", tt, (double)now);
    //double ticks = (double)now + timespec_to_ticks(t, freq);
    double then = (double)now.tms_stime + (double)now.tms_utime + t.tv_sec*100+t.tv_nsec/10000000;

    while((double)(now.tms_stime + now.tms_utime) < then){
       for (int i = 0; i < 10000; i++){
            times(&now);
        }
    }

}

int main(int argc, char* argv[]) {
    if (strcmp(argv[1], "A") == 0) {
        if(strcmp(argv[2], "SLEEP")==0){
            unsigned int duration = 1;
            sleep(duration);
        }
        if(strcmp(argv[2], "USLEEP")==0){
            unsigned int duration = 1000000;
            usleep(duration);
        }
        if(strcmp(argv[2], "NANOSLEEP")==0){
            struct timespec duration = {1,0};
            struct timespec rem = {0,0};
            nanosleep(&duration, &rem);
        }
        if(strcmp(argv[2], "BUSY_CLOCK")==0){
            struct timespec duration = {1,0};
            busy_wait(duration);
        }
        if(strcmp(argv[2], "BUSY_TIMES")==0){
            struct timespec duration = {1,0};
            busy_times(duration);
        }
    }
    if (strcmp(argv[1], "B") == 0) {
        int N = 10*1000*1000;
        if (strcmp(argv[2], "RESOLUTION") == 0) {
            int ns_max = 50;
            int histogram[ns_max];
            memset(histogram, 0, sizeof(int)*ns_max);

            if (strcmp(argv[3], "RDTSC") == 0) {
                int t1;
                int t2;
                for (int i = 0; i < N; i++) {
                    t1 = __rdtsc();
                    t2 = __rdtsc();
                    int ns = (int)((double)(t2 - t1) / FREQUENCY_NS);

                    if (ns >= 0 && ns < ns_max) {
                        histogram[ns]++;
                    }
                }
            }
            if (strcmp(argv[3], "CLOCK") == 0) {
                struct timespec t1;
                struct timespec t2;
                struct timespec ret;
                for (int i = 0; i < N; i++) {
                    clock_gettime(CLOCK_MONOTONIC, &t1);
                    clock_gettime(CLOCK_MONOTONIC, &t2);
                    ret = timespec_sub(t2,t1);
                    int ns = ret.tv_nsec + ret.tv_sec * BILLION;

                    if (ns >= 0 && ns < ns_max) {
                        histogram[ns]++;
                    }
                }
            }
            if (strcmp(argv[3], "TIMES") == 0) {
                struct tms t1;
                struct tms t2;
                for (int i = 0; i < N; i++) {
                    times(&t1);
                    times(&t2);
                    int tt1 = (int)((double)t1.tms_utime + (double)t1.tms_stime);
                    int tt2 = (int)((double)t2.tms_utime + (double)t2.tms_stime);
                    int ns = (int)((double)(tt2 - tt1) * BILLION / 100.0);
                    //printf("%d, %d, %d\n", tt1, tt2, ns);

                    if (ns >= 0 && ns < ns_max) {
                        histogram[ns]++;
                    }
                }
            }

            for (int i = 0; i < ns_max; i++) {
                printf("%d\n", histogram[i]);
            }
        }
        if (strcmp(argv[2], "LATENCY") == 0) {
            if (strcmp(argv[3], "RDTSC") == 0) {
                int now = __rdtsc();
                int then;
                for (int i = 0; i < N; i++) {
                    then = __rdtsc();
                }
                printf("%dns\n", (int)(((double)(then - now) / FREQUENCY_NS) / (double)N));
            }
            if (strcmp(argv[3], "CLOCK") == 0) {
                struct timespec now;
                struct timespec then;
                clock_gettime(CLOCK_MONOTONIC, &now);
                for (int i = 0; i < N; i++) {
                    clock_gettime(CLOCK_MONOTONIC, &then);
                }
                struct timespec ret = timespec_sub(then, now);
                printf("%dns\n", (int)((ret.tv_nsec + ret.tv_sec * BILLION) / (double)N));
            }
            if (strcmp(argv[3], "TIMES") == 0) {
                struct tms then;
                struct tms now;
                times(&now);
                for (int i = 0; i < N; i++) {
                    times(&then);
                }
                int nowtime = (int)((double)now.tms_utime + (double)now.tms_stime);
                int thentime = (int)((double)then.tms_utime + (double)then.tms_stime);
                printf("%dns\n", (int)((double)(thentime - nowtime)*BILLION/(double)(100.0 * N)));
            }
        }
    }
    if (strcmp(argv[1], "C") == 0) {
        int N = 10*1000*1000;
        int ns_max = 1000;
        int histogram[ns_max];
        memset(histogram, 0, sizeof(int)*ns_max);

        struct timespec first;
        struct timespec last;
        for (int i = 0; i < N; i++) {
            clock_gettime(CLOCK_MONOTONIC, &first);
            sched_yield();
            clock_gettime(CLOCK_MONOTONIC, &last);

            struct timespec ret = timespec_sub(last, first);
            int ns = ret.tv_nsec + ret.tv_sec * BILLION;
            if (ns >= 0 && ns < ns_max) {
                histogram[ns]++;
            }

        }

        for (int i = 0; i < ns_max; i++) {
            printf("%d\n", histogram[i]);
        }

    }

    return 0;

    // nanosleep

    //busy_wait clock_gettime
    //busy_wait times
}