#define LWTP_C

typedef struct {
    pthread_t* threads;
    _Atomic (int) count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_mutex_t cond_mutex;
    job_handler_t job;
    pthread_mutex_t block;
} lwt_pool_t;

#include "lwtp.h"
#include <stdatomic.h>


// executed by each worker
void* _worker(void* pool_p) {
    lwt_pool_t* pool = (lwt_pool_t*)pool_p;
    job_handler_t job;

    while(1) {
        // keep this locked whenever a worker is waiting
        pthread_mutex_lock(&(pool->block));

        // wait for a job
        pthread_mutex_lock(&(pool->cond_mutex));
        pthread_cond_wait(&(pool->cond), &(pool->cond_mutex));

        pthread_mutex_lock(&(pool->mutex));

        pool->count++;

        job = pool->job;
        pool->job = NULL;

        pthread_mutex_unlock(&(pool->mutex));

        // unblock whoever gave us the job to let them know we took it
        pthread_mutex_unlock(&(pool->block));

        // execute the job
        job();

        pthread_mutex_lock(&(pool->mutex));
        pool->count--;
        pthread_mutex_unlock(&(pool->mutex));
    }
}

int lwtp_start(lwt_pool_t* pool, job_handler_t job) {
    pthread_mutex_lock(&(pool->mutex));
    pool->job = job;

    pthread_mutex_unlock(&(pool->mutex));

    pthread_cond_signal(&(pool->cond));

    // there's 2 cases here
    // we block until the worker unlocks this OR
    // we get it immediately because there was no worker available
    pthread_mutex_lock(&(pool->block));

    // if there was no worker available, the job will not be taken and we should return error
    pthread_mutex_lock(&(pool->mutex));
    job_handler_t job_cp = pool->job;
    pthread_mutex_unlock(&(pool->mutex));

    if(job_cp) {
        // no one took the job :(
        return -1;
    }

    return 0;
}

void lwtp_wait(lwt_pool_t* pool) {
    while(pool->count) {};
}

void lwtp_wait_count(lwt_pool_t* pool, int count) {
    while(pool->count >= count) {};
}

int lwtp_create(lwt_pool_t* pool, uint32_t num_threads) {
    if(pthread_mutex_init(&(pool->mutex), NULL)) {
        // failed to initialize mutex
        return -1;
    }

    if(pthread_mutex_init(&(pool->cond_mutex), NULL)) {
        // failed to initialize mutex
        return -1;
    }

    if(pthread_mutex_init(&(pool->cond), NULL)) {
        // failed to initialize mutex
        return -1;
    }

    if(pthread_cond_init(&(pool->cond), NULL)) {
        // failed to initialize mutex
        return -1;
    }

    pool->threads = malloc(sizeof(pthread_t) * num_threads);

    if(!pool->threads) {
        // malloc failed to allocate
        return -1;
    }

    pool->count = 0;

    // start all the threads
    for(uint32_t i = 0; i < num_threads; i++) {
        if(pthread_create(&(pool->threads[i]), NULL, &_worker, pool)) {
            // failure creating thread
            return -1;
        }
    }

    return 0;
}
