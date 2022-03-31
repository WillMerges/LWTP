#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "lwtp.h"

#define NUM_RUNS  10
#define POOL_SIZE 1000000

void test_func(void* t) {
    // do some floating point stuff
    float f = 0.056213123;
    float g = f * 0.983920;

    double e = f * g / 100202;
    e /= 5.6;

    float h = e * f * g * 2;

    void* hh = malloc(1000000);
    void* gg = malloc(1000000);

    memcpy(hh, gg, 1000000);

    free(hh);
    free(gg);

    usleep(100);
}

void print_time(struct timespec* start, struct timespec* end) {
    time_t sec = end->tv_sec - start->tv_sec;
    long nsec = end->tv_nsec - start->tv_nsec;

    if(nsec < 0) {
        sec--;
        nsec = 1000000000 - nsec;
    }

    printf("sec: %lu, nsec: %li\n", sec, nsec);
}

void main() {
    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i = 0; i < NUM_RUNS; i++) {
        test_func(NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("completed baseline (no thread pool) run\n");
    print_time(&start, &end);
    printf("\n");

    lwt_pool_t pool;
    job_t job = {&test_func, NULL};
    lwtp_create(&pool, POOL_SIZE);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i = 0; i < NUM_RUNS; i++) {
        lwtp_start(&pool, &job);
    }
    lwtp_wait(&pool);
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("completed run with thread pool\n");
    print_time(&start, &end);
    printf("\n");
}
