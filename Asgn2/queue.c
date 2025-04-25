/*
 * Description: This file contains queue structure utils.
 * Author: iwong12
 * Date: 2025-04-23
 */

#include <stdio.h>
#include "lwp.h"
#include <stdlib.h>

/*
 * Description:
 *   Initializes a queue structure.
 * Parameters:
 *   Pointer to a global queue.
 * Returns:
 *   Nothing.
 */
Queue *startup(int lib) {
    Queue *q = (Queue *)malloc(sizeof(Queue));
    if (q == NULL) {
        perror("allocating global queue");
        return NULL;
    }
    q->sen = malloc(sizeof(context));
    if (q->sen == NULL) {
        perror("error mallocing sen");
        free(q);
        return NULL;
    }
    q->sen->tid = NO_THREAD;
    if (lib == TRUE) {
        q->sen->lib_one = q->sen;
        q->sen->lib_two = q->sen;
        q->length = 0;
    } else {
        q->sen->sched_one = q->sen;
        q->sen->sched_two = q->sen;
        q->length = 0;
    }
    return q;
}

/*
 * Description:
 *   Adds a thread to the end of a queue.
 * Parameters:
 *   The queue being mutated and the thread to add.
 * Returns:
 *   Nothing.
 */
void enqueue(Queue *q, thread t, int lib) {
    if (lib == TRUE) {
        t->lib_one = q->sen;
        t->lib_two = q->sen->lib_two;
        q->sen->lib_two->lib_one = t;
        q->sen->lib_two = t;
    } else {
        t->sched_one = q->sen;
        t->sched_two = q->sen->sched_two;
        q->sen->sched_two->sched_one = t;
        q->sen->sched_two = t;
    }
    q->length++;
}

/*
 * Description:
 *   Removes a given thread from a queue.
 * Parameters:
 *   The queue to mutate and the thread to remove.
 * Returns:
 *   Nothing.
 */
void dequeue(Queue *q, thread t, int lib) {
    thread cur = q->sen;
    if (lib == TRUE) {
        while (cur->lib_one != q->sen && cur != t) {
            cur = cur->lib_one;
        }
        if (cur == t) {
            cur->lib_two->lib_one = cur->lib_one;
            cur->lib_one->lib_two = cur->lib_two;
            cur->lib_one = NULL;
            cur->lib_two = NULL;
            q->length--;
        }
    } else {
        while (cur->sched_one != q->sen && cur != t) {
            cur = cur->sched_one;
        }
        if (cur == t) {
            cur->sched_two->sched_one = cur->sched_one;
            cur->sched_one->sched_two = cur->sched_two;
            cur->sched_one = NULL;
            cur->sched_two = NULL;
            q->length--;
        }
    }
}

/*
 * Description:
 *   Shuts down a queue.
 * Parameters:
 *   The queue to shut down.
 * Returns:
 *   Nothing.
 */
void shutdown(Queue *q) {
    free(q->sen);
        free(q);
}
