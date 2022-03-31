/*
*   Light Weight Thread Pools
*
*   @author: Will Merges
*/
#ifndef LWTP_H
#define LWTP_H

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>


typedef void (*job_handler_t)(void*);

typedef struct {
    job_handler_t func;
    void* arg;
} job_t;

typedef struct {
    pthread_t* threads;
    int num_threads;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int start;
    int end;
    job_t** jobs;
} lwt_pool_t;

// create a thread pool with 'num_threads' threads
// returns 0 on success, anything else on error
int lwtp_create(lwt_pool_t* pool, int num_threads);

// destroy a thread pool
int lwtp_destroy(lwt_pool_t*);

// start a job on a thread
// TODO add a blocking start, use condition variables to wake up parent
int lwtp_start(lwt_pool_t* pool, job_t* job);

// wait for all jobs to complete
// NOTE: blocking
void lwtp_wait(lwt_pool_t* pool);

// wait until there are less than 'count' jobs executing
// NOTE: blocking
void lwtp_wait_count(lwt_pool_t* pool, int count);

#endif
