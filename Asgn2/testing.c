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
    int i, num[5] = {0, 1, 2, 3, 4};
    for (i = 0; i < 5; i++) {
        lwp_create(test1, num + i);
    }
    lwp_start();
    for (i = 0; i < 5; i++) {
        int status;
        lwp_wait(&status);
        printf("%d\n", status);
    }
    return 0;
}