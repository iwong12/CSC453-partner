/*
 * Description: This file contains the round-robin scheduler library.
 * Author: iwong12
 * Date: 2025-04-22
 */

#include "lwp.h"
#include <stdio.h>
#include <stdlib.h>

struct scheduler rr = {rr_init, rr_shutdown, rr_admit, rr_remove, rr_next, rr_qlen};
scheduler sched = &rr;
Queue ready_help = {NULL, 0};
Queue *ready = &ready_help;


/*
 * Description:
 *   Initializes the round-robin scheduler.
 * Parameters:
 *   None.
 * Returns:
 *   Nothing.
 */
void rr_init(void) {
    startup(ready);
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
    shutdown(ready);
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
    if (ready->sen == NULL) {
        rr_init();
    }
    if (ready->sen == NULL) {  // second check after rr_init() to see if fail
        perror("error initializing rr scheduler");
        return;
    }
    enqueue(ready, new);
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
    if (ready->sen == NULL) {
        perror("cannot remove from uninitialized scheduler");
        return;
    }
    if (victim == NULL) {
        perror("cannot remove NULL thread");
        return;
    }
    dequeue(ready, victim);
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

int main(void) {
    thread t1 = malloc(sizeof(thread));
    sched->admit(t1);
    thread next = sched->next();
    int ql = sched->qlen();
    sched->remove(next);
    t1 = malloc(sizeof(thread));
    sched->admit(t1);
    sched->shutdown();
}
