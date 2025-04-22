/*
 * Description: This file contains the round-robin scheduler library.
 * Author: iwong12
 * Date: 2025-04-10
 */

#include "lwp.h"
#include <stdio.h>
#include <stdlib.h>

struct scheduler rr = {rr_init, rr_shutdown, rr_admit, rr_remove, rr_next, rr_qlen};
scheduler RoundRobin = &rr;
Queue *queue;


/*
 * Description:
 *   Initializes the round-robin scheduler.
 * Parameters:
 *   None.
 * Returns:
 *   Nothing.
 */
void rr_init(void) {
    queue = malloc(sizeof(Queue));
    if (queue == NULL) {
        perror("error mallocing queue");
        return;
    }
    queue->sentinel = malloc(sizeof(Node));
    if (queue->sentinel == NULL) {
        perror("error mallocing sentinel");
        free(queue);
        queue = NULL;
        return;
    }
    queue->length = 0;
    queue->sentinel->next = queue->sentinel;
    queue->sentinel->prev = queue->sentinel;
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
    if (queue == NULL) {
        perror("cannot shutdown uninitialized scheduler");
        return;
    }
    Node *cur = queue->sentinel->next;
    while (cur != queue->sentinel) {
        Node *old = cur;
        cur = cur->next;
        free(old);
    }
    free(queue->sentinel);
    free(queue);
    queue = NULL;
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
    if (queue == NULL) {
        rr_init();
    }
    if (queue == NULL) {  // second check after rr_init() to see if fail
        perror("error initializing rr scheduler");
        return;
    }
    Node *new_thread = malloc(sizeof(Node));
    if (new_thread == NULL) {
        perror("error mallocing new thread");
        return;
    }
    new_thread->thread = new;
    new_thread->next = queue->sentinel;
    new_thread->prev = queue->sentinel->prev;
    queue->sentinel->prev->next = new_thread;
    queue->sentinel->prev = new_thread;
    queue->length++;
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
    if (queue == NULL) {
        perror("cannot remove from uninitialized scheduler");
        return;
    }
    if (victim == NULL) {
        perror("cannot remove NULL thread");
        return;
    }
    Node *cur = queue->sentinel;
    while (cur->next != queue->sentinel && cur->thread != victim) {
        cur = cur->next;
    }
    if (cur->thread == victim) {
        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;
        free(cur);
        queue->length--;
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
    if (queue == NULL || queue->sentinel->next == queue->sentinel) {
        perror("cannot get next thread from uninitialized/empty scheduler");
        return NULL;
    }
    return queue->sentinel->next->thread;
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
    if (queue == NULL) {
        perror("cannot get length of uninitialized scheduler");
        return -1;
    }
    return queue->length;
}

int main(void) {
    thread t1 = malloc(sizeof(thread));
    RoundRobin->admit(t1);
    thread next = RoundRobin->next();
    int ql = RoundRobin->qlen();
    RoundRobin->remove(next);
    RoundRobin->admit(t1);
    RoundRobin->shutdown();
    free(t1);
}
