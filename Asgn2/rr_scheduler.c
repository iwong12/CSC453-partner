/*
 * Description: This file contains the round-robin scheduler library.
 * Author: iwong12
 * Date: 2025-04-22
 */

#include "lwp.h"
#include <stdio.h>
#include <stdlib.h>

struct scheduler rr = {rr_init, rr_shutdown, rr_admit, rr_remove, rr_next, rr_qlen};
scheduler RoundRobin = &rr;
Queue *ready;
Queue *zombie;
Queue *waiting;
Queue *all;


/*
 * Description:
 *   Initializes the round-robin scheduler.
 * Parameters:
 *   None.
 * Returns:
 *   Nothing.
 */
void rr_init(void) {
    ready = malloc(sizeof(Queue));
    if (ready == NULL) {
        perror("error mallocing queue");
        return;
    }
    ready->sen = malloc(sizeof(thread));
    if (ready->sen == NULL) {
        perror("error mallocing sen");
        free(ready);
        ready = NULL;
        return;
    }
    ready->length = 0;
    ready->sen->sched_one = ready->sen;
    ready->sen->sched_two = ready->sen;
}


/*
 * Description:
 *   Shuts down the round-robin scheduler.
 * Parameters:
 *   None.
 * Returns:
 *   Nothing.
 */
void rr_shutdown(void) {
    if (ready == NULL) {
        perror("cannot shutdown uninitialized scheduler");
        return;
    }
    thread cur = ready->sen->sched_one;
    while (cur != ready->sen) {
        thread old = cur;
        cur = cur->sched_one;
        free(old);
    }
    free(ready->sen);
    free(ready);
    ready = NULL;
}

/*
 * Description:
 *   Adds a new thread to the scheduler.
 * Parameters:
 *   The new thread to add.
 * Returns:
 *   Nothing.
 */
void rr_admit(thread new) {
    if (new == NULL) {
        perror("cannot add NULL thread");
        return;
    }
    if (ready == NULL) {
        rr_init();
    }
    if (ready == NULL) {  // second check after rr_init() to see if fail
        perror("error initializing rr scheduler");
        return;
    }
    new->sched_one = ready->sen;
    new->sched_two = ready->sen->sched_two;
    ready->sen->sched_two->sched_one = new;
    ready->sen->sched_two = new;
    ready->length++;
}

/*
 * Description:
 *   Removes a thread from the scheduler.
 * Parameters:
 *   The thread to remove.
 * Returns:
 *   Nothing.
 */
void rr_remove(thread victim) {
    if (ready == NULL) {
        perror("cannot remove from uninitialized scheduler");
        return;
    }
    if (victim == NULL) {
        perror("cannot remove NULL thread");
        return;
    }
    thread cur = ready->sen;
    while (cur->sched_one != ready->sen && cur != victim) {
        cur = cur->sched_one;
    }
    if (cur == victim) {
        cur->sched_two->sched_one = cur->sched_one;
        cur->sched_one->sched_two = cur->sched_two;
        free(cur);
        ready->length--;
    }
}

/*
 * Description:
 *   Gets the next thread to run.
 * Parameters:
 *   None.
 * Returns:
 *   The thread to run next, or NULL if there are no more threads.
 */
thread rr_next(void) {
    if (ready == NULL || ready->sen->sched_one == ready->sen) {
        perror("cannot get next thread from uninitialized/empty scheduler");
        return NULL;
    }
    return ready->sen->sched_one;
}

/*
 * Description:
 *   Retrieves the number of runnable threads.
 * Parameters:
 *   None.
 * Returns:
 *   The number of runnable threads.
 */
int rr_qlen(void) {
    if (ready == NULL) {
        perror("cannot get length of uninitialized scheduler");
        return -1;
    }
    return ready->length;
}

// int main(void) {
//     thread t1 = malloc(sizeof(thread));
//     RoundRobin->admit(t1);
//     thread next = RoundRobin->next();
//     int ql = RoundRobin->qlen();
//     RoundRobin->remove(next);
//     t1 = malloc(sizeof(thread));
//     RoundRobin->admit(t1);
//     RoundRobin->shutdown();
// }
