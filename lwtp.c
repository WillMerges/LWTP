#include <signal.h>

// #define LWTP_C

#include "lwtp.h"


// executed by each worker
void* _worker(void* pool_p) {
    lwt_pool_t* pool = (lwt_pool_t*)pool_p;
    job_handler_t job;
    void* arg;

    while(1) {
        // wait for a job
        int ret = pthread_mutex_lock(&(pool->mutex));
        pool->ready++;

        pthread_cond_wait(&(pool->cond), &(pool->mutex));

        pool->ready--;

        if(!pool->job) {
            // someone already took the job and beat us to it
            pthread_mutex_unlock(&(pool->mutex));
            continue;
        }

        pool->count++;

        job = pool->job;
        arg = pool->arg;

        pool->job = NULL;

        pthread_mutex_unlock(&(pool->mutex));

        // execute the job
        job(arg);

        pthread_mutex_lock(&(pool->mutex));
        pool->count--;
        pthread_mutex_unlock(&(pool->mutex));
    }
}

int lwtp_start(lwt_pool_t* pool, job_handler_t job, void* arg) {
    pthread_mutex_lock(&(pool->mutex));
    if(!pool->ready) {
        // no workers available yet
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }

    pool->job = job;
    pool->arg = arg;
    pthread_mutex_unlock(&(pool->mutex));

    pthread_cond_signal(&(pool->cond));

    int exit = 0;
    while(!exit) {
        pthread_mutex_lock(&(pool->mutex));
        if(!pool->job) {
            exit = 1;
        }
        pthread_mutex_unlock(&(pool->mutex));
    }

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

    // if(pthread_mutex_init(&(pool->cond_mutex), NULL)) {
    //     // failed to initialize mutex
    //     return -1;
    // }

    if(pthread_cond_init(&(pool->cond), NULL)) {
        // failed to initialize condition variable
        return -1;
    }

    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);

    if(!pool->threads) {
        // malloc failed to allocate
        return -1;
    }

    pool->num_threads = num_threads;
    pool->count = 0;
    pool->job = NULL;
    pool->arg = NULL;
    pool->ready = 0;

    // start all the threads
    for(uint32_t i = 0; i < num_threads; i++) {
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

    // if(pthread_mutex_destroy(&(pool->cond_mutex))) {
    //     return -1;
    // }

    if(pthread_cond_destroy(&(pool->cond))) {
        return -1;
    }
}
