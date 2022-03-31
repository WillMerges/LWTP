#include <signal.h>
#include <unistd.h>

// #define LWTP_C

#include "lwtp.h"


// executed by each worker
void* _worker(void* pool_p) {
    lwt_pool_t* pool = (lwt_pool_t*)pool_p;
    int job;

    // wait for a job
    int ret = pthread_mutex_lock(&(pool->mutex));
    // pool->ready++;

    while(1) {
        pthread_cond_wait(&(pool->cond), &(pool->mutex));

        job = pool->start;
        pool->start++;
        pool->start %= pool->num_threads;
        pool->count++;

        pthread_mutex_unlock(&(pool->mutex));

        // execute the job
        pool->jobs[job]->func(pool->jobs[job]->arg);

        pthread_mutex_lock(&(pool->mutex));

        // mark job as complete
        pool->jobs[job] = NULL;
        pool->count--;
    }
}

int lwtp_start(lwt_pool_t* pool, job_t* job) {
    pthread_mutex_lock(&(pool->mutex));

    if(pool->count == pool->num_threads) {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }

    pool->jobs[pool->end] = job;

    pool->end++;
    pool->end %= pool->num_threads;

    pthread_mutex_unlock(&(pool->mutex));

    pthread_cond_signal(&(pool->cond));

    return 0;
}

void lwtp_wait(lwt_pool_t* pool) {
    int c = 1;
    while(c) {
        pthread_mutex_lock(&(pool->mutex));
        c = pool->count;
        pthread_mutex_unlock(&(pool->mutex));
    };
}

void lwtp_wait_count(lwt_pool_t* pool, int count) {
    int c = count + c;
    while(c >= count) {
        pthread_mutex_lock(&(pool->mutex));
        c = pool->count;
        pthread_mutex_unlock(&(pool->mutex));
    };
}

int lwtp_create(lwt_pool_t* pool, int num_threads) {
    if(pthread_mutex_init(&(pool->mutex), NULL)) {
        // failed to initialize mutex
        return -1;
    }

    if(pthread_cond_init(&(pool->cond), NULL)) {
        // failed to initialize condition variable
        return -1;
    }

    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);

    if(!pool->threads) {
        // malloc failed to allocate
        return -1;
    }

    pool->jobs = (job_t**)malloc(sizeof(job_t*) * num_threads);

    if(!pool->jobs) {
        // malloc failed to allocate
        return -1;
    }

    pool->num_threads = num_threads;
    pool->count = 0;
    pool->start = 0;
    pool->end = 0;

    // start all the threads and zero jobs
    for(uint32_t i = 0; i < num_threads; i++) {
        pool->jobs[i] = NULL;
        if(pthread_create(&(pool->threads[i]), NULL, _worker, pool)) {
            // failure creating thread
            return -1;
        }
    }

    return 0;
}


int lwtp_destroy(lwt_pool_t* pool) {
    // kill all of our threads
    // we give them a SIGINT to be nice

    for(int i = 0; i < pool->num_threads; i++) {
        if(pthread_kill(pool->threads[i], SIGINT)) {
            return -1;
        }
    }

    // destroy all mutexes and condition variables

    if(pthread_mutex_destroy(&(pool->mutex))) {
        return -1;
    }

    if(pthread_cond_destroy(&(pool->cond))) {
        return -1;
    }
}
