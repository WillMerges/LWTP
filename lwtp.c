#define LWTP_C

typedef struct {
    pthread_t* threads;
    uint32_t count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_mutex_t cond_mutex;
    job_handler_t job;
} lwt_pool_t;

#include "lwtp.h"


// executed by each worker
void* _worker(void* pool_p) {
    lwt_pool_t* pool = (lwt_pool_t*)pool_p;
    job_handler_t job;

    while(1) {
        // wait for a job
        pthread_mutex_lock(&(pool->cond_mutex));
        pthread_cond_wait(&(pool->cond), &(pool->cond_mutex));

        pthread_mutex_lock(&(pool->mutex));

        pool->count++;

        job = pool->job;
        pool->job = NULL;

        pthread_mutex_unlock(&(pool->mutex));

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

    uint32_t curr = pool->count;

    pthread_mutex_unlock(&(pool->mutex));

    pthread_cond_signal(&(pool->cond));

    // TODO some way to know that the job was started

    return 0;
}

void lwtp_wait(lwt_pool_t* pool) {
    uint32_t count = 1;
    while(count) {
        pthread_mutex_lock(&(pool->mutex));
        count = pool->count;
        pthread_mutex_unlock(&(pool->mutex));
    }
}

void lwtp_wait_count(lwt_pool_t* pool, uint32_t count) {
    uint32_t c = target + 1;
    while(c >= target) {
        pthread_mutex_lock(&(pool->mutex));
        c = pool->count;
        pthread_mutex_unlock(&(pool->mutex));
    }
}

int lwtp_create(lwt_pool_t* pool, uint32_t num_threads) {
    pool->threads = malloc(sizeof(pthread_t) * num_threads);

    if(!pool->threads) {
        // malloc failed to allocate
        return -1;
    }

    if(pthread_mutex_init(&(pool->mutex), NULL)) {
        // failed to initialize mutex
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
