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
void startup(Queue *q) {
    q->sen = malloc(sizeof(thread));
    if (q->sen == NULL) {
        perror("error mallocing sen");
        return;
    }
    if (q == all) {
        q->sen->lib_one = q->sen;
        q->sen->lib_two = q->sen;
        q->length = 0;
    } else {
        q->sen->sched_one = q->sen;
        q->sen->sched_two = q->sen;
        q->length = 0;
    }
}

/*
 * Description:
 *   Adds a thread to the end of a queue.
 * Parameters:
 *   The queue being mutated and the thread to add.
 * Returns:
 *   Nothing.
 */
void enqueue(Queue *q, thread t) {
    if (q == all) {
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
void dequeue(Queue *q, thread t) {
    thread cur = q->sen;
    if (q == all) {
        while (cur->lib_one != q->sen && cur != t) {
            cur = cur->lib_one;
        }
        if (cur == t) {
            cur->lib_two->lib_one = cur->lib_one;
            cur->lib_one->lib_two = cur->lib_two;
            q->length--;
        }
    } else {
        while (cur->sched_one != q->sen && cur != t) {
            cur = cur->sched_one;
        }
        if (cur == t) {
            cur->sched_two->sched_one = cur->sched_one;
            cur->sched_one->sched_two = cur->sched_two;
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
    q = NULL;
}