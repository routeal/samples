#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include "threadpool.h"

#undef DPRINTF
#define DPRINTF(M, ...)

typedef struct threadworker threadworker_t;

struct threadworker {
    ccl_thread_callback   func;
    void                 *arg;
    threadworker_t       *next;
};

struct threadpool {
    /* pool characteristics */
    unsigned int          num_threads;
    unsigned int          max_queue_size;
    unsigned int          dont_block_when_full;

    /* pool state */
    pthread_t            *threads;
    threadworker_t       *queue_head;
    threadworker_t       *queue_tail;
    unsigned int          cur_queue_size;
    int                   queue_closed;
    int                   shutdown;

    /* pool synchronization */
    pthread_mutex_t       queue_lock;
    pthread_cond_t        queue_not_empty;
    pthread_cond_t        queue_not_full;
    pthread_cond_t        queue_empty;
};

static void *tpool_worker_func(void *);

threadpool_t *threadpool_create(unsigned int num_worker_threads, unsigned int max_queue_size, unsigned int dont_block_when_full)
{
    unsigned int i;
    unsigned int num;
    threadpool_t *tp;
   
    if ((tp = (threadpool_t *) malloc(sizeof(threadpool_t))) == (void*)0) {
        DPRINTF("tpool_init malloc %s\n", strerror(errno));
        return (void*)0;
    }

    if ((tp->threads = (pthread_t *) malloc(sizeof(pthread_t) * (unsigned long) num_worker_threads)) == (void*)0) {
        DPRINTF("tpool_init malloc %s\n", strerror(errno));
        free(tp);
        return (void*)0;
    }

    tp->num_threads = num_worker_threads;
    tp->max_queue_size = max_queue_size;
    tp->dont_block_when_full = dont_block_when_full;
    tp->cur_queue_size = 0;
    tp->queue_head = (void*)0; 
    tp->queue_tail = (void*)0;
    tp->queue_closed = 0;  
    tp->shutdown = 0; 

    pthread_mutex_init(&(tp->queue_lock), (void*)0);
    pthread_cond_init(&(tp->queue_not_empty), (void*)0);
    pthread_cond_init(&(tp->queue_not_full), (void*)0);
    pthread_cond_init(&(tp->queue_empty), (void*)0);
  
    /* create worker threads to be pooled*/
    for (i = 0, num = 0; i != num_worker_threads; i++) {
        int rtn;
        if ((rtn = pthread_create(&(tp->threads[num]), (void*)0, tpool_worker_func, (void *) tp)) != 0) {
            DPRINTF("tpool_init pthread_create %s\n", strerror(rtn));
            continue;
        }
        DPRINTF("pthread_create %d\n", (int)tp->threads[num]);
        num++;
    }

    if (num == 0) {
        // all pthread_create failed
        free(tp->threads);
        free(tp);
        return (void*)0;
    }

    /*
    CONSOLE_LOG_INFO("thread pool size=%d, max thread queue size=%d\n", 
                     num_worker_threads, max_queue_size);
    */

    return tp;
}

int threadpool_add(threadpool_t *tpool, ccl_thread_callback func, void *arg)
{
    threadworker_t *worker;

    pthread_mutex_lock(&(tpool->queue_lock));

    /* no space and this caller doesn't want to wait */
    if ((tpool->cur_queue_size == tpool->max_queue_size) &&
        tpool->dont_block_when_full) {
        pthread_mutex_unlock(&(tpool->queue_lock));
        return 1;
    }

    while ((tpool->cur_queue_size == tpool->max_queue_size) &&
           (!(tpool->shutdown || tpool->queue_closed))) {
        pthread_cond_wait(&(tpool->queue_not_full), &(tpool->queue_lock));
    }

    /* the pool is in the process of being destroyed */
    if (tpool->shutdown || tpool->queue_closed) {
        pthread_mutex_unlock(&(tpool->queue_lock));
        return 1;
    }

    /* allocate work structure */
    if ((worker = (threadworker_t *) malloc(sizeof(threadworker_t))) == (void*)0) {
        DPRINTF("tpool_add_work malloc %s\n", strerror(errno));
        return -1;
    }
  
    worker->func = func;
    worker->arg = arg;
    worker->next = (void*)0;

    DPRINTF("adder: adding an item %d\n", (int) (long long) worker->func);

    if (tpool->cur_queue_size == 0) {
        tpool->queue_tail = tpool->queue_head = worker;

        DPRINTF("%s", "adder: queue == 0, waking all workers\n");

        pthread_cond_broadcast(&(tpool->queue_not_empty));
    } else {
        tpool->queue_tail->next = worker;
        tpool->queue_tail = worker;
    }

    tpool->cur_queue_size++; 

    pthread_mutex_unlock(&(tpool->queue_lock));

    return 0;
}

void threadpool_destroy(threadpool_t *tpool, int finish)
{
    unsigned int i;
    threadworker_t *tmp;

    pthread_mutex_lock(&(tpool->queue_lock));

    /* Is a shutdown already in progress? */
    if (tpool->queue_closed || tpool->shutdown) {
        pthread_mutex_unlock(&(tpool->queue_lock));
        return;
    }

    tpool->queue_closed = 1;

    /* If the finish flag is set, wait for workers to 
       drain queue */ 
    if (finish == 1) {
        while (tpool->cur_queue_size != 0) {
            pthread_cond_wait(&(tpool->queue_empty), &(tpool->queue_lock));
        }
    }

    tpool->shutdown = 1;

    pthread_mutex_unlock(&(tpool->queue_lock));

    /* Wake up any workers so they recheck shutdown flag */
    pthread_cond_broadcast(&(tpool->queue_not_empty));
  
    pthread_cond_broadcast(&(tpool->queue_not_full));

    /* Wait for workers to exit */
    for (i=0; i < tpool->num_threads; i++) {
        pthread_join(tpool->threads[i], (void*)0);
    }

    /* Now free pool structures */
    free(tpool->threads);

    while (tpool->queue_head != (void*)0) {
        tmp = tpool->queue_head->next; 
        tpool->queue_head = tpool->queue_head->next;
        free(tmp);
    }

    free(tpool); 
}

void *tpool_worker_func(void *arg)
{
    threadpool_t *tpool = (threadpool_t *) arg; 
    threadworker_t *tworker;
    int state;
        
    for (;;) {
        /* Check queue for work */ 
        pthread_mutex_lock(&(tpool->queue_lock));

        while ((tpool->cur_queue_size == 0) && (!tpool->shutdown)) {
            DPRINTF("worker %d: I'm sleeping again\n", (int)pthread_self());

            pthread_cond_wait(&(tpool->queue_not_empty), &(tpool->queue_lock));
        }

        DPRINTF("worker %d: I'm awake\n", (int) pthread_self());

        /* Has a shutdown started while i was sleeping? */
        if (tpool->shutdown == 1) {
            pthread_mutex_unlock(&(tpool->queue_lock));
            pthread_exit((void*)0);
        }

        /* Get to work, dequeue the next item */ 
        tworker = tpool->queue_head;
        tpool->cur_queue_size--;

        if (tpool->cur_queue_size == 0) {
            tpool->queue_head = tpool->queue_tail = (void*)0;
        } else {
            tpool->queue_head = tworker->next;
        }

        DPRINTF("worker %d: dequeing item %d\n", 
                (int) pthread_self(), (int) (long long) tworker->func);

        /* Handle waiting add_work threads */
        if ((!tpool->dont_block_when_full) &&
            (tpool->cur_queue_size ==  (tpool->max_queue_size - 1))) 

            pthread_cond_broadcast(&(tpool->queue_not_full));

        /* Handle waiting destroyer threads */
        if (tpool->cur_queue_size == 0)
            pthread_cond_signal(&(tpool->queue_empty));

        pthread_mutex_unlock(&(tpool->queue_lock));
      
        /* Do this work item */
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
        (*(tworker->func))(tworker->arg);
        pthread_setcancelstate(state, (void*)0);

        free(tworker);
    }
}
