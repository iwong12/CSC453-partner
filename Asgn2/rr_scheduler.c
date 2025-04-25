/*
 * Description: This file contains the round-robin scheduler library.
 * Author: iwong12
 * Date: 2025-04-22
 */

#include "lwp.h"
#include <stdio.h>
#include <stdlib.h>

scheduler sched = NULL;
Queue *ready = NULL;


/*
 * Description:
 *   Initializes the round-robin scheduler.
 * Parameters:
 *   None.
 * Returns:
 *   Nothing.
 */
void rr_init(void) {
    sched = malloc(sizeof(struct scheduler));
    if (sched == NULL) {
        perror("error allocating scheduler");
        return;
    }
    sched->init = rr_init;
    sched->shutdown = rr_shutdown;
    sched->admit = rr_admit;
    sched->remove = rr_remove;
    sched->next = rr_next;
    sched->qlen = rr_qlen;
    if (ready == NULL) {
        ready = startup(FALSE);
        if (ready == NULL) {
            perror("error initializing queues");
            free(sched);
        }
    }
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
    if (ready != NULL && ready->length == 0) {
        shutdown(ready);
    }
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
    if (sched == NULL) {
        rr_init();
    }
    if (sched == NULL) {  // second check after rr_init() to see if fail
        perror("error initializing rr scheduler");
        return;
    }
    enqueue(ready, new, FALSE);
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
    if (sched == NULL) {
        rr_init();
    }
    if (sched == NULL) {  // second check after rr_init() to see if fail
        perror("error initializing rr scheduler");
        return;
    }
    if (victim == NULL) {
        perror("cannot remove NULL thread");
        return;
    }
    dequeue(ready, victim, FALSE);
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
    if (sched == NULL) {
        rr_init();
        return NULL;
    }
    if (rr_qlen() < 1) {
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
    if (sched == NULL) {
        rr_init();
        return 0;
    }
    return ready->length;
}
