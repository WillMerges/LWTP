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

typedef void (*job_handler_t)();

#ifndef LWTP_C
typedef struct {} lwt_pool_t;
#endif

// create a thread pool with 'num_threads' threads
// returns 0 on success, anything else on error
int lwtp_create(lwt_pool_t* pool, uint32_t num_threads);


// destroy a thread pool
int lwtp_destroy(lwt_pool_t*);

// start a job on a thread
int lwtp_start(lwt_pool_t* pool, job_handler_t job);

// wait for all jobs to complete
// NOTE: blocking
void lwtp_wait(lwt_pool_t* pool);

// wait until there are less than 'count' jobs executing
// NOTE: blocking
void lwtp_wait_count(lwt_pool_t* pool, uint32_t count) {

#endif
