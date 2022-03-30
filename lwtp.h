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

#ifndef LWTP_C
typedef void (*job_handler_t)(void*);
typedef struct {} lwt_pool_t;
#endif

// create a thread pool with 'num_threads' threads
// returns 0 on success, anything else on error
int lwtp_create(lwt_pool_t* pool, int num_threads);

// destroy a thread pool
int lwtp_destroy(lwt_pool_t*);

// start a job on a thread
// TODO add a blocking start, use condition variables to wake up parent
int lwtp_start(lwt_pool_t* pool, job_handler_t job, void* arg);

// wait for all jobs to complete
// NOTE: blocking
void lwtp_wait(lwt_pool_t* pool);

// wait until there are less than 'count' jobs executing
// NOTE: blocking
void lwtp_wait_count(lwt_pool_t* pool, int count);

#endif
