/*
 * Description: Library for lightweight process management.
 * Authors: iwong12, ckira
 * Date: 2025-04-22
 */

#define DEFAULT_STACK 8388608
#define BOUND 16
#define BYTES 8

#include "lwp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/mman.h>

long stacksize = -1;
unsigned long threads = 0;
thread running = NULL;
Queue global = {NULL, 0};
Queue dead = {NULL, 0};
Queue wait = {NULL, 0};
Queue *all = &global;
Queue *zombie = &dead;
Queue *blocked = &wait;


/*
 * Description:
 *   Calls the given lwpfunction with the given argument,
 *   then calls lwp_exit() with its return value.
 * Paramters:
 *   The lwpfun to be called along with its arguments.
 * Returns:
 *   Nothing.
 */
static void lwp_wrap(lwpfun fun, void *arg) {
    int rval = fun(arg);
    lwp_exit(rval);
}

/*
 * Description:
 *   Checks to make sure resources exist and
 *   initializes if the don't.
 * Paramters:
 *   Nothing.
 * Returns:
 *   0 on success, -1 on error.
 */
int check_init(void) {
    if (all->sen == NULL) {
        all = startup(TRUE);
        if (all == NULL) {
            perror("initializing all");
            return -1;
        }
    }
    if (zombie->sen == NULL) {
        zombie = startup(FALSE);
        if (zombie == NULL) {
            perror("initializing zombie");
            return -1;
        }
    }
    if (blocked->sen == NULL) {
        blocked = startup(FALSE);
        if (blocked == NULL) {
            perror("initializing blocked");
            return -1;
        }
    }

    if (sched == NULL) {
        rr_init();
    }
    if (sched == NULL) {  // second check after rr_init() to see if fail
        perror("error initializing rr scheduler");
        return -1;
    }
    return 0;
}

/*
 * Description:
 *   Sets the stack size for LWPs based on system resource limits.
 * Parameters:
 *   None.
 * Returns:
 *   0 on success, -1 on error.
 */
int set_stack_size(void) {
    long pgsize = sysconf(_SC_PAGESIZE);
    if (pgsize == -1) {
        perror("sysconf");
        return -1;
    }
    struct rlimit rlim;
    if (getrlimit(RLIMIT_STACK, &rlim) == -1) {
        perror("getrlimit");
        return -1;
    }
    if (rlim.rlim_cur != RLIM_INFINITY) {
        stacksize = (long)rlim.rlim_cur;
    } else {
        stacksize = DEFAULT_STACK;
    }
    if (stacksize % pgsize != 0) {
        stacksize += pgsize - stacksize % pgsize;
    }
    return 0;
}

/*
 * Description:
 *   Creates a new lightweight process which executes the given function
 *   with the given argument.
 * Parameters:
 *   The function to execute in the new thread, along with its arguments.
 * Returns:
 *   The (lightweight) thread id of the new thread
 *   or NO THREAD if the thread cannot be created.
 */
tid_t lwp_create(lwpfun function, void *argument) {
    if (check_init() == -1) {
        perror("initialization error");
        return NO_THREAD;
    }

    if  (function == NULL) {
        perror("cannot create thread with NULL function");
        return NO_THREAD;
    }
    if (stacksize == -1) {
        if (set_stack_size() == -1) {
            perror("error setting stack size");
            return NO_THREAD;
        }
    }
    /* check params */

    thread new = malloc(sizeof(context));
    if (new == NULL) {
        perror("error mallocing new thread");
        return NO_THREAD;
    }

    new->tid = ++threads;

    new->stack = mmap(NULL, stacksize,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1, 0);
    if (new->stack == MAP_FAILED) {
        perror("error mmaping new thread stack");
        free(new);
        return NO_THREAD;
    }
    /* create new thread var and new stack for it */

    unsigned long offset = ((unsigned long)(((char *)new->stack)
                            + stacksize)) % BOUND;
    *(unsigned long *)((char *)new->stack + stacksize - offset - BYTES * 2)
        = (unsigned long)lwp_wrap;
    /*  going to the spot in bytes (with stacksize and offset).
        then it divides by the size of a long, then -2 for correct
        stack spot
    */

    new->stacksize = stacksize;

    new->state.rdi = (unsigned long)function;
    new->state.rsi = (unsigned long)argument;
    new->state.rbp = (unsigned long)((char *)new->stack +
                      stacksize - offset - BYTES * 3);
    /* set correct spot in stack for swap_rfiles to read properly */

    new->state.fxsave = FPU_INIT;

    new->status = LWP_LIVE;

    enqueue(all, new, TRUE);
    sched->admit(new);
    /* add to all and scheduler queue */

    return new->tid;
}

/*
 * Description:
 *   Starts the LWP system. Converts the calling thread into a LWP
 *   and lwp_yield()s to whichever thread the scheduler chooses.
 * Parameters:
 *   None.
 * Returns:
 *   Nothing.
 */
void lwp_start(void) {
    if (check_init() == -1) {
        perror("initialization error");
        return;
    }

    thread new = malloc(sizeof(context));
    if (new == NULL) {
        perror("error mallocing new thread");
        return;
    }
    /* creates new context to save for current thread */
    running = new;
    new->tid = ++threads;
    new->stack = NULL;
    new->status = LWP_LIVE;

    enqueue(all, new, TRUE);
    sched->admit(new);
    /* add main thread to all and scheduler */
    lwp_yield();
    /* yield handles the rest */
}

/*
 * Description:
 *   Yields control to another LWP. Which one depends on the scheduler.
 *   Saves the current LWP's context, picks the next one, restores that
 *   thread's context, and returns. If there is no next thread,
 *   terminates the program.
 * Parameters:
 *   None.
 * Returns:
 *   Nothing.
 */
void lwp_yield(void) {
    if (check_init() == -1) {
        perror("initialization error");
        return;
    }

    thread current = running;
    /* find current and reset running */

    thread later = sched -> next();

    if (later == NULL) {
        exit(1);
    }
    /* if next val does not exist, exit */

    sched -> remove(later);
    sched -> admit(later);
    /* put it to the back of the queue */

    running = later;

    swap_rfiles(&(current -> state), &(later -> state));
    /* change state */
}

/*
 * Description:
 *   Terminates the current LWP and yields to whichever thread the
 *   scheduler chooses. lwp_exit() does not return.
 * Parameters:
 *   An int whose lowest 8 bits are the termination status
 *   of the lightweight thread to be terminated.
 * Returns:
 *   Nothing.
 */
void lwp_exit(int exitval) {
    if (check_init() == -1) {
        perror("initialization error");
        return;
    }

    sched->remove(running);
    /* get current thread and remove from scheduler */
    enqueue(zombie, running, FALSE);
    running->status = MKTERMSTAT(LWP_TERM, exitval);
    /* set status and put into zombie list to be deallocted */

    if (blocked -> length > 0){
        thread revived = blocked -> sen -> sched_one;
        dequeue(blocked, revived, FALSE);
        sched -> admit(revived);
        /* put thread in waited list back into scheduler */
        revived -> exited = running;
        /* set exited of the waited thread to exited thread */
    }

    lwp_yield();
}

/*
 * Description:
 *   Waits for a thread to terminate, deallocates its resources,
 *   and reports its termination status if status is non-NULL.
 * Parameters:
 *   A pointer to an int that will hold the thread's termination status.
 * Returns:
 *   The tid of the terminated thread or NO_THREAD.
 */
tid_t lwp_wait(int *status) {
    if (check_init() == -1) {
        perror("initialization error");
        return NO_THREAD;
    }

    if (zombie -> length < 1){
        sched -> remove(running);
        enqueue(blocked, running, FALSE);
        if (sched -> qlen() < 1){
            return NO_THREAD;
        }
        /* if there are no more threads, just ret */
        lwp_yield();
    }
    /* wait for zombie to show up */

    thread delete = zombie -> sen -> sched_one;
    *status = delete -> status;
    tid_t final = delete -> tid;
    /* find the oldest to delete and get the id */
    dequeue(zombie, delete, FALSE);
    dequeue(all, delete, TRUE);

    if (delete -> stack != NULL){
        if (munmap(delete->stack, delete->stacksize) == -1) {
            perror("error munmap");
            return NO_THREAD;
        };
    }
    /* if main thread stack, do not deallocate */

    free(delete);
    return final;
    /* unqueue it and free it all */
}

/*
 * Description:
 *   Retrieves the tid of the current LWP.
 * Parameters:
 *   None.
 * Returns:
 *   The tid of the calling LWP or NO_THREAD if not called
 *   by a LWP.
 */
tid_t lwp_gettid(void) {
    return running->tid;
    /* the element at back of queue is running process */
}

/*
 * Description:
 *   Retrieves the thread associated with the given tid.
 * Parameters:
 *   A tid.
 * Returns:
 *   The thread corresponding to the given thread ID, or NULL
 *   if the ID is invalid.
 */
thread tid2thread(tid_t tid) {
    if (check_init() == -1) {
        perror("initialization error");
        return NULL;
    }

    int test = FALSE;

    thread current = all -> sen -> lib_one;
    while (test == FALSE){
        if (current == (all -> sen)){
            current = NULL;
            test = TRUE;
        }
        else if ((current -> tid) == tid){
            test = TRUE;
        } else {
            current = current -> lib_one;
        }
    }
    /* iterates through all till is done or finds */

    return current;
}

/*
 * Description:
 *   Causes the LWP package to use the given scheduler to choose
 *   the next process to run. Transfers all threads from the old
 *   scheduler to the new one in next() order. If scheduler is NULL
 *   the library returns to round-robin scheduling.
 * Parameters:
 *   A new scheduler to choose which LWP to run.
 * Returns:
 *   Nothing.
 */
void lwp_set_scheduler(scheduler new) {
    if (check_init() == -1) {
        perror("initialization error");
        return;
    }

    if (new == NULL) {
        new = malloc(sizeof(struct scheduler));
        new->init = rr_init;
        new->shutdown = rr_shutdown;
        new->admit = rr_admit;
        new->remove = rr_remove;
        new->next = rr_next;
        new->qlen = rr_qlen;
    }
    if (sched == new) {
        perror("cannot set scheduler to itself");
        return;
    }

    new->init();

    Queue *temp = startup(FALSE);
    if (temp == NULL) {
        perror("cannot allocate temp context");
        return;
    }

    while (sched->qlen() > 0) {
        thread cur = sched->next();
        sched->remove(cur);
        enqueue(temp, cur, FALSE);
    }
    while (temp->length > 0) {
        thread cur = temp->sen->sched_one;
        dequeue(temp, cur, FALSE);
        new->admit(cur);
    }

    sched->shutdown();
    sched = new;
}

/*
 * Description:
 *   Retrieves the current scheduler.
 * Parameters:
 *   None.
 * Returns:
 *   A pointer to the current scheduler.
 */
scheduler lwp_get_scheduler(void) {
    return sched;
}
