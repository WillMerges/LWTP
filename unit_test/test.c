#include <stdio.h>
#include <unistd.h>
#include "lwtp.h"

void test(void* t) {
    printf("job executed\n");
    fflush(stdout);
}

lwt_pool_t pool;

int main() {
    lwtp_create(&pool, 10);
    job_t job = {&test, NULL};


    while(1) {
        lwtp_start(&pool, &job);
        lwtp_start(&pool, &job);
        lwtp_start(&pool, &job);
        lwtp_wait(&pool);
        printf("\n");
        sleep(2);
    };
}
