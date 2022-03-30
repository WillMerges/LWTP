#include <stdio.h>
#include "lwtp.h"

void test(void* t) {
    printf("job executed\n");
}

// for some reason this can't go on the stack
// it may be a compiler optimization seeing the spinning while at the end of main and returning
lwt_pool_t pool;

int main() {

    lwtp_create(&pool, 1);
    
    int ret = lwtp_start(&pool, &test, NULL);
    printf("%i\n", ret);

    while(1) {};
}
