#include <stdio.h>
#include "lwtp.h"

void test(void* t) {
    printf("job executed\n");
}

int main() {
    lwt_pool_t pool;

    lwtp_create(&pool, 10);

    // lwtp_start(&pool, &test, NULL);

    while(1) {};
}
