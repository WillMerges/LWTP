#include <stdio.h>
#include <unistd.h>
#include "lwtp.h"

void test(void* t) {
    printf("job executed\n");
    fflush(stdout);
}

lwt_pool_t pool;

int main() {
    lwtp_create(&pool, 1000);

    while(1) {
        lwtp_start(&pool, &test, NULL);
        lwtp_start(&pool, &test, NULL);
        lwtp_start(&pool, &test, NULL);
        lwtp_start(&pool, &test, NULL);
        lwtp_wait(&pool);
        printf("\n");
        sleep(2);
    };
}
