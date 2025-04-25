//
// Created by Ian on 4/24/2025.
//

#include <stdio.h>
#include "lwp.h"

int test1(void *arg) {
    printf("%d: hello!\n", *(int *)arg);
    lwp_yield();
    lwp_exit(0);
    return 0;
}

int main(void) {
    int i, num[10] = {0, 1, 2, 3, 4,5,6,7,8,9};
    for (i = 0; i < 10; i++) {
        lwp_create(test1, num + i);
    }
    lwp_start();
    for (i = 0; i < 10; i++) {
        int status;
        lwp_wait(&status);
        printf("%d: %d\n", i, status);
    }
    return 0;
}