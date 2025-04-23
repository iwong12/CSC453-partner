//
// Created by Ian on 4/21/2025.
//

#define DEFAULT_STACK 8388608

#include "lwp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <math.h>
#include <sys/mman.h>

long stacksize = -1;
int threads = 0;


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

    new->stacksize = stacksize;

    new->state.rdi = (unsigned long)argument;
    new->state.rbp = (unsigned long)new->stack;
    new->state.rsp = (unsigned long)new->stack + 1;
    new->state.fxsave = FPU_INIT;

    new->status = LWP_LIVE;

    RoundRobin->admit(new);

    return new->tid;
}

/*
 * Description:
 *   Starts the LWP system. Converts the calling thread into a LWP
 *   and lwp yield()s to whichever thread the scheduler chooses.
 * Parameters:
 *   None.
 * Returns:
 *   Nothing.
 */
void lwp_start(void) {
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
    return 0;
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
    return 0;
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
    return NULL;
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
void lwp_set_scheduler(scheduler sched) {
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
    return NULL;
}

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
    int rval;
    rval = fun(arg);
    lwp_exit(rval);
}

int main(void) {
    printf("Hello World!\n");
}
