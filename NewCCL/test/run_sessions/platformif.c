#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#pragma warning( push, 3 ) // Saves the current warning level and sets it to 4
#include <windows.h>
#pragma warning( pop )     // Restores the old warning level
#include <process.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#endif

#include "platformif.h"
#include "util.h"
#ifndef _WIN32
#include "threadpool.h"
#endif

extern void my_sleep(unsigned int second);

/* timestamp in milli seconds */
static uint64_t current_unix_time(void)
{
#ifdef _WIN32
    uint64_t v;
    GetSystemTimeAsFileTime((FILETIME*)&v);
    return (uint64_t) (v / 10000 - 11644473600000LL);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t) (tv.tv_sec * 1000LL + tv.tv_usec / 1000);
#endif
}

static void print_log(const char *message)
{
    if (message != NULL) {
        fprintf(stderr, "%s\n", message);
    }
}

static void get_platform_metadata (ccl_platform_metadata_t *metadata)
{
    if (metadata == NULL) return;
    ccl_strncpy(metadata->device_brand, sizeof(metadata->device_brand), "Macbook Pro", strlen("Macbook Pro"));
    ccl_strncpy(metadata->device_manufacturer, sizeof(metadata->device_manufacturer), "Apple", strlen(""));
    ccl_strncpy(metadata->device_model, sizeof(metadata->device_model), "Retina 2013", strlen("Retina 2013"));
    metadata->device_type = CCL_DEVICE_TYPE_DESKTOP;
    ccl_strncpy(metadata->framework_name, sizeof(metadata->framework_name), "gstreamer", strlen("gstreamer"));
    ccl_strncpy(metadata->framework_version, sizeof(metadata->framework_version), "1.0", strlen("1.0"));
    ccl_strncpy(metadata->operating_system_name, sizeof(metadata->operating_system_name), "MacOS", strlen("MacOS"));
    ccl_strncpy(metadata->operating_system_version, sizeof(metadata->operating_system_version), "10.9", strlen("10.9"));
}

static void load_data(ccl_load_callback func)
{
    const char *clid = "{\"clid\":\"1976252.6667240.1322884.12345678\"}";
    func(CCL_SUCCESS, clid, (unsigned int) strlen(clid));
}

static void save_data(const char *data, unsigned int length, ccl_save_callback func)
{
    (void) data;
    (void) length;
    func(CCL_SUCCESS, NULL, 0);
}

typedef struct http_request
{
    ccl_http_callback func;
    void * data;
} http_request_t;

#ifdef _WIN32
static unsigned __stdcall http_thread(void *arg)
#else
static void http_thread(void *arg)
#endif
{
    const char *json = "{\"gw\":\"https://localhost:1443/\",\"hbi\":5,\"seq\":0,\"err\":\"ok\",\"clid\":\"1976252.6667240.1322884.12345678\",\"t\":\"CwsServerResponse\",\"sid\":2,\"svt\":1401484708201,\"slg\":false}";

    http_request_t *req = (http_request_t *) arg;

    my_sleep(5);

    req->func(CCL_SUCCESS, req->data, json, (unsigned int) strlen(json));

    free(req);

#ifdef _WIN32
    _endthreadex(0);
    return 0;
#endif
}

#ifndef _WIN32
static threadpool_t *mThreadPool = NULL;
#endif

static void request_http(const char *url, const char *content_type,
                         const char *request, unsigned int request_length, unsigned int timeout,
                         ccl_http_callback func, void * http_data)
{
    http_request_t *req;

    (void) url;
    (void) content_type;
    (void) request;
    (void) request_length;
    (void) timeout;

    req = (http_request_t *) malloc(sizeof(http_request_t));
    req->func = func;
    req->data = http_data;

#ifdef _WIN32
    //_beginthreadex(0, 0, http_thread, req, 0, 0);
    CloseHandle((HANDLE)_beginthreadex(0, 0, http_thread, req, 0, 0));
#else
    threadpool_add(mThreadPool, http_thread, req);
#endif
}

#ifdef _WIN32

typedef struct ccl_timer ccl_timer_t;
struct ccl_timer {
  HANDLE handle;
  ccl_timer_callback func;
  void *args;
};

static void CALLBACK timerProc(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    ccl_timer_t *timer = (ccl_timer_t *) lpParameter;
    (void) TimerOrWaitFired;
    timer->func(timer->args);
}

static void * create_timer(ccl_timer_callback func, void *args,
                           unsigned int initial_time, unsigned int interval)
{
    ccl_timer_t *timer;
    if ((timer = (ccl_timer_t *) malloc(sizeof(ccl_timer_t))) == NULL) {
        return NULL;
    }
    memset(timer, 0, sizeof(ccl_timer_t));

    timer->func = func;
    timer->args = args;

    CreateTimerQueueTimer(&timer->handle, NULL, timerProc, timer,
                          initial_time+100, interval, WT_EXECUTEDEFAULT);
    return (void *) timer;
}

static void destroy_timer(void *ptimer)
{
    ccl_timer_t *timer = (ccl_timer_t *) ptimer;
    if (!timer) return;
    DeleteTimerQueueTimer(NULL, (HANDLE) timer->handle, INVALID_HANDLE_VALUE);
    free(timer);
}

#else

typedef struct ccl_timer ccl_timer_t;
struct ccl_timer {
    pthread_t thread;
    pthread_mutex_t lock;
    pthread_cond_t reschedule;
    ccl_timer_callback func;
    void *args;
    uint64_t value;
    uint64_t interval;
    int shutdown;
};

static void *timer_thread(void *arg)
{
    ccl_timer_t *timer = (ccl_timer_t *) arg;
    pthread_mutex_lock(&(timer->lock));
    for (;;) {
        while (timer->shutdown == 0 && timer->value == 0) {
            pthread_cond_wait(&(timer->reschedule), &(timer->lock));
        }
        if (timer->shutdown == 0) {
            ldiv_t d = ldiv((long) timer->value, (long long) (1000000));
            struct timespec ts = { d.quot, d.rem * (1000000000 / 1000000) };
            if (pthread_cond_timedwait(&timer->reschedule, &timer->lock, &ts) == 0) {
                continue;
            }
        }
        if (timer->shutdown) {
            break;
        }
        if (timer->interval == 0) {
            timer->value = 0;
        }
        pthread_mutex_unlock(&timer->lock);
        if (timer->func != (void*)0) {
            timer->func(timer->args);
        }
        pthread_mutex_lock(&(timer->lock));
        if (timer->interval == 0) {
            continue;
        }
        timer->value += timer->interval;
    }
    pthread_mutex_unlock(&(timer->lock));
    return (void*) 0;
}

static void * create_timer(ccl_timer_callback func, void *args, unsigned int initial_time, unsigned int interval)
{
    int rtn;
    ccl_timer_t *timer;
    if ((timer = (ccl_timer_t *) malloc(sizeof(ccl_timer_t))) == (void*)0) {
        return (void*)0;
    }
    memset(timer, 0, sizeof(ccl_timer_t));
    pthread_mutex_init(&timer->lock, (void*)0);
    pthread_cond_init(&timer->reschedule, (void*)0);
    timer->func = func;
    timer->args = args;
    timer->value = 0;
    timer->interval = 0;
    timer->shutdown = 0;
    if ((rtn = pthread_create(&(timer->thread), (void*)0,
                              timer_thread, (void *) timer)) != 0) {
        fprintf(stderr, "ccl_timer_create pthread_create %s\n", strerror(rtn));
        pthread_cond_destroy(&timer->reschedule);
        pthread_mutex_destroy(&timer->lock);
        free(timer);
        return (void*)0;
    }
    pthread_mutex_lock(&timer->lock);
    timer->value = (initial_time + current_unix_time()) * 1000;
    timer->interval = (uint64_t) (interval * 1000);
    pthread_cond_signal(&timer->reschedule);
    pthread_mutex_unlock(&timer->lock);
    return (void *) timer;
}

static void destroy_timer(void *ptimer)
{
    if (ptimer) {
        ccl_timer_t *timer = (ccl_timer_t *) ptimer;
        timer->shutdown = 1;
        pthread_cond_signal(&timer->reschedule);
        pthread_join(timer->thread, (void*)0);
        pthread_cond_destroy(&timer->reschedule);
        pthread_mutex_destroy(&timer->lock);
        free (timer);
    }
}

#endif

static int generate_rand(void)
{
    return rand();
}

static void * mutex_init (void)
{
#ifdef _WIN32
    return CreateMutex (NULL, FALSE, NULL);
#else
    pthread_mutex_t *lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    if (lock) pthread_mutex_init(lock, NULL);
    return (void *) lock;
#endif
}

static void mutex_lock (void * lock)
{
#ifdef _WIN32
    WaitForSingleObject((HANDLE) lock, INFINITE);
#else
    if (lock) pthread_mutex_lock((pthread_mutex_t *)lock);
#endif
}

static void mutex_unlock (void *lock)
{
#ifdef _WIN32
    ReleaseMutex((HANDLE) lock);
#else
    if (lock) pthread_mutex_unlock((pthread_mutex_t *)lock);
#endif
}

static void mutex_destroy (void *lock)
{
#ifdef _WIN32
    CloseHandle ((HANDLE) lock);
#else
    if (lock) pthread_mutex_destroy((pthread_mutex_t *)lock);
    free(lock);
#endif
}

static void * rwlock_init (void)
{
#ifdef _WIN32
  return NULL;
  //    return CreateMutex (NULL, FALSE, NULL);
#else
    pthread_rwlock_t *rwlock = (pthread_rwlock_t *) malloc(sizeof(pthread_rwlock_t));
    if (rwlock) pthread_rwlock_init(rwlock, NULL);
    return rwlock;
#endif
}

static void rwlock_rdlock (void *rwlock)
{
#ifdef _WIN32
    WaitForSingleObject((HANDLE) rwlock, INFINITE);
#else
    if (rwlock) pthread_rwlock_rdlock(rwlock);
#endif
}

static void rwlock_wrlock (void *rwlock)
{
#ifdef _WIN32
    WaitForSingleObject((HANDLE) rwlock, INFINITE);
#else
    if (rwlock) pthread_rwlock_wrlock(rwlock);
#endif
}

static void rwlock_unlock (void *rwlock)
{
#ifdef _WIN32
    ReleaseMutex((HANDLE) rwlock);
#else
    if (rwlock) pthread_rwlock_unlock(rwlock);
#endif
}

static void rwlock_destroy (void *rwlock)
{
#ifdef _WIN32
    CloseHandle ((HANDLE) rwlock);
#else
    if (rwlock) pthread_rwlock_destroy(rwlock);
    free(rwlock);
#endif
}

static ccl_platform_t pif = {
    save_data,
    load_data,
    request_http,
    print_log,
    current_unix_time,
    get_platform_metadata,
    create_timer,
    destroy_timer,
    generate_rand,
    mutex_init,
    mutex_lock,
    mutex_unlock,
    mutex_destroy,
    rwlock_init,
    rwlock_rdlock,
    rwlock_wrlock,
    rwlock_unlock,
    rwlock_destroy,
};

void platform_destroy(void)
{
#ifndef _WIN32
    threadpool_destroy(mThreadPool, 1);
#endif
}

const ccl_platform_t *platform_create(unsigned int session_size)
{
#ifdef _WIN32
    (void) session_size;
#else
    srand((unsigned int) current_unix_time());
    mThreadPool = threadpool_create(session_size*2, session_size*2, 0);
#endif
    return &pif;
}
