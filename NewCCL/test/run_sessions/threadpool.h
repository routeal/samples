#ifndef __TPOOL_H_
#define __TPOOL_H_

#include <ccl.h>

typedef struct threadpool threadpool_t;

typedef void (*ccl_thread_callback)(void *);

extern threadpool_t *threadpool_create(unsigned int num_threads, unsigned int max_queue_size, unsigned int dont_block_when_full);

extern void threadpool_destroy(threadpool_t *tpool, int finish);

extern int threadpool_add(threadpool_t *tpool, ccl_thread_callback func, void *arg);

#endif /* __TPOOL_H_ */
