#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <inttypes.h>

#include "ccl.h"
#include "jvalue.h"

typedef struct test_player
{
  ccl_session *session;
  int state;
  int count;
} test_player;

static const char *program_name = "run_sessions";

static unsigned int monitor_interval = 200;

static const char *get_cdn(void);

static const char *get_message(void);

/* return a random integer between min and max */
static unsigned int randr(unsigned int min, unsigned int max)
{
  return (unsigned int) rand() % (max - min + 1) + min;
}

static void sleep_random(unsigned int second)
{
  if (second > 1) second = randr(1, second);
  sleep(second);
}

static unsigned long long int current_time(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (unsigned long long int) (tv.tv_sec * 1000LL + tv.tv_usec / 1000);
}

/** prints the log to console */
static int print_log(const char *log, unsigned int logsiz)
{
  (void) logsiz;
  if (log) printf("%s\n", log);
  return 0;
}

/** saves the config */
static int save_config(const char *buf, unsigned int bufsiz)
{
  (void) buf;
  (void) bufsiz;
  return 0;
}

/** loads the config */
static int load_config(char *buf, unsigned int bufsiz)
{
  strncpy(buf, "{\"clid\":\"1976252.6667240.1322884.12345678\"}", bufsiz);
  return 0;
}

static void *hb_thread(void *arg)
{
  int sid = (int) arg;

  jvalue *obj = jobject_create();
  jobject_put_string(obj, "t", "CwsServerResponse");
  jobject_put_string(obj, "err", "ok");
  jobject_put_string(obj, "clid", "1976252.6667240.1322884.12345678");
  jobject_put_int(obj, "sid", sid);

  char *json = jvalue_write(obj, 0);
  //printf("%s\n", json);
  jvalue_destroy(obj);

  ccl_parse_hearbeat(json, (unsigned int) strlen(json));

  free(json);

  return 0;
}

/** sends the heartbeat to the Conviva server */
static int send_heartbeat(const char *url, const char *content_type,
                          const char *heartbeat, unsigned int heartbeat_size)
{
  (void) url;
  (void) content_type;
  (void) heartbeat_size;

  jvalue *value = jvalue_read(heartbeat, 0);
  int sid = jobject_get_int(value, "sid", 0);
  jvalue_destroy(value);

  //printf("%s\n", heartbeat);
  sleep_random(2);
  pthread_t thread;
  pthread_create(&thread, 0, hb_thread, (void*) (long long) sid);
  return 0;
}

typedef struct ccl_timer
{
  pthread_t thread;
  pthread_mutex_t lock;
  pthread_cond_t reschedule;
  ccl_timer_callback func;
  void *args;
  uint64_t value;
  uint64_t interval;
  int shutdown;
} ccl_timer_t;

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
    if (timer->func != 0) {
      timer->func(timer->args);
    }
    pthread_mutex_lock(&(timer->lock));
    if (timer->interval == 0) {
      continue;
    }
    timer->value += timer->interval;
  }
  pthread_mutex_unlock(&(timer->lock));
  return 0;
}

/** creates and starts a timer */
static void * create_timer(ccl_timer_callback func, void *data,
                           unsigned int initial_time, unsigned int interval)
{
  int rtn;
  ccl_timer_t *timer;
  if ((timer = (ccl_timer_t *) malloc(sizeof(ccl_timer_t))) == 0) return 0;
  memset(timer, 0, sizeof(ccl_timer_t));
  pthread_mutex_init(&timer->lock, 0);
  pthread_cond_init(&timer->reschedule, 0);
  timer->func = func;
  timer->args = data;
  timer->value = 0;
  timer->interval = 0;
  timer->shutdown = 0;
  if ((rtn = pthread_create(&(timer->thread), 0, timer_thread, (void *) timer)) != 0) {
    fprintf(stderr, "ccl_timer_create pthread_create %s\n", strerror(rtn));
    pthread_cond_destroy(&timer->reschedule);
    pthread_mutex_destroy(&timer->lock);
    free(timer);
    return 0;
  }
  pthread_mutex_lock(&timer->lock);
  timer->value = (initial_time + current_time()) * 1000;
  timer->interval = (uint64_t) (interval * 1000);
  pthread_cond_signal(&timer->reschedule);
  pthread_mutex_unlock(&timer->lock);
  return (void *) timer;
}

/** destroys the timer */
static void destroy_timer(void *timer_handle)
{
  if (timer_handle) {
    ccl_timer_t *timer = (ccl_timer_t *) timer_handle;
    timer->shutdown = 1;
    pthread_cond_signal(&timer->reschedule);
    pthread_join(timer->thread, (void*)0);
    pthread_cond_destroy(&timer->reschedule);
    pthread_mutex_destroy(&timer->lock);
    free(timer);
  }
}

static ccl_platform_interface this_platform_interface = {
  print_log,
  save_config,
  load_config,
  send_heartbeat,
  create_timer,
  destroy_timer
};

static ccl_platform_interface *platform_interface = &this_platform_interface;

static int get_playhead_time(void *arg)
{
  static int ret = 0;
  test_player *player = (test_player *) arg;
  if (player) {
    if (player->state == CCL_PLAYING) {
      ret = player->count;
      player->count += monitor_interval;
    }
  }
  return ret;
}

static int get_buffer_length(void *arg)
{
  (void) arg;
  return (int) randr(1000, 10000);
}

static int get_framerate(void *arg)
{
  (void) arg;
  return (int) randr(1000, 10000);
}

static ccl_player this_player_status_informer = {
  get_playhead_time,
  get_buffer_length,
  get_framerate,
};

static ccl_player *player_status_informer = &this_player_status_informer;

static void test_ad(ccl_session *session)
{
  ccl_session_suspend(session);
  sleep_random(5);
  ccl_session_resume(session);
}

static void test_set_state(test_player *player, int state)
{
  switch (state) {
    case CCL_PLAYING:
      fprintf(stderr, "%s: setState: %s\n", program_name, "PLAYING");
      break;
    case CCL_STOPPED:
      fprintf(stderr, "%s: setState: %s\n", program_name, "STOPPED");
      break;
    case CCL_PAUSED:
      fprintf(stderr, "%s: setState: %s\n", program_name, "PAUSED");
      break;
    case CCL_BUFFERING:
      fprintf(stderr, "%s: setState: %s\n", program_name, "BUFFERING");
      break;
    case CCL_NOTMONITORED:
      fprintf(stderr, "%s: setState: %s\n", program_name, "NOTMONITORED");
      break;
    case CCL_UNKNOWN:
      fprintf(stderr, "%s: setState: %s\n", program_name, "UNKNOWN");
      break;
  }
  ccl_session_update_playback_state(player->session, state);
}

static void test_set_bitrate(test_player *player)
{
  int bitrate = (int) randr(1000, 10000);
  fprintf(stderr, "%s: setBitrate: bitrate=%d\n", program_name, bitrate);
  ccl_session_update_bitrate(player->session, bitrate);
}

static void test_set_cdn(test_player *player)
{
  const char *cdn = get_cdn();
  fprintf(stderr, "%s: setCdn: cdn=%s\n", program_name, cdn);
  ccl_session_update_cdn(player->session, cdn);
}

static void test_set_log(test_player *player)
{
  const char *log = get_message();
  fprintf(stderr, "%s: log: %s\n", program_name, log);
  ccl_session_log(player->session, log);
}

static void test_set_error(test_player *player)
{
  const char *error = get_message();
  fprintf(stderr, "%s: setError: %s\n", program_name, error);
  ccl_session_error(player->session, error, 1);
}

static void test_set_duration(test_player *player)
{
  int duration = (int) randr(1, 100000);
  fprintf(stderr, "%s: setDuration: duration=%d\n", program_name, duration);
  ccl_session_update_duration(player->session, duration);
}

static void test_set_encoded_framerate(test_player *player)
{
  int framerate = (int) randr(20, 100);
  fprintf(stderr, "%s: setFramerate: framerate=%d\n", program_name, framerate);
  ccl_session_update_framerate(player->session, framerate);
}

static void test_session_send_event(ccl_session *session)
{
  ccl_tag *t = ccl_create_tag();
  ccl_set_tag(t, "key1", "value1");
  ccl_set_tag(t, "key2", "value2");
  ccl_session_send_event(session, "attr", t);
  ccl_destroy_tag(t);
}

static void test_send_event(void)
{
  ccl_tag *t = ccl_create_tag();
  ccl_set_tag(t, "key1", "value1");
  ccl_set_tag(t, "key2", "value2");
  ccl_send_event("attr", t);
  ccl_destroy_tag(t);
}

static ccl_metadata *test_content(void)
{
  ccl_tag *t = ccl_create_tag();
  ccl_set_tag(t, "testkey", "testvalue");

  ccl_metadata *m = ccl_create_metadata();
  ccl_set_metadata_string(m, ASSET_NAME, "test video");
  ccl_set_metadata_string(m, CDN_NAME, get_cdn());
  ccl_set_metadata_string(m, VIEWER_ID, "1234");
  ccl_set_metadata_string(m, PLAYER_NAME, "test player");
  ccl_set_metadata_string(m, MEDIA_URL, "http://a.b.com/movie.avi");
  ccl_set_metadata_int(m, MEDIA_IS_LIVE, (int) randr(0, 1));
  ccl_set_metadata_int(m, MEDIA_BITRATE, (int) randr(1000, 10000));
  ccl_set_metadata_tag(m, t);

  ccl_destroy_tag(t);

  return m;
}

static void run_session(int iteration)
{
  ccl_session *session;
  ccl_metadata *info;
  test_player *player;
  int count = iteration;

  info = test_content();

  session = ccl_session_create(info, 0);

  ccl_destroy_metadata(info);

  player = (test_player *) malloc(sizeof(test_player));
  memset(player, 0, sizeof(test_player));

  player->session = session;
  player->state = CCL_UNKNOWN;
  player->count = 0;

  ccl_session_attach(session, player_status_informer, player);

  /* pre-roll */
  test_ad(session);

  test_send_event();

  while (count--) {
    int state = CCL_UNKNOWN;
    int c = count % 6;
    switch (c) {
      case 0:
        state = CCL_STOPPED;
        break;
      case 1:
        state = CCL_PLAYING;
        break;
      case 2:
        state = CCL_BUFFERING;
        break;
      case 3:
        state = CCL_PAUSED;
        break;
      case 4:
        state = CCL_NOTMONITORED;
        break;
      case 5:
        state = CCL_UNKNOWN;
        break;
    }

    player->state = state;

    test_set_state(player, state);
    test_set_bitrate(player);
    test_set_cdn(player);
    test_set_log(player);
    test_set_error(player);
    test_set_duration(player);
    test_set_encoded_framerate(player);
    test_session_send_event(session);

    sleep_random(4);
  }

  test_send_event();

  /* post-roll */
  test_ad(session);

  ccl_session_detach(session);

  ccl_session_end(session);

  ccl_session_destroy(session);

  free(player);
}

static void *worker(void *p)
{
  run_session((int) (intptr_t) p);
  return NULL;
}

int main(int argc, const char *argv[])
{
  const char *gateway_url = "http://localhost:8080";
  const char *key = "b914e177d9f9d20a835cf84915c381affe24f163";

  int i;
  unsigned int session_size = 1; /* default session size */
  unsigned int iteration = 10; /* default number of iteration */
  unsigned int loglevel = 4;
  unsigned int heartbeat_interval = 2*1000;
  ccl_metadata *settings, *platform;
  pthread_t *thread;

  for (i = 1; i < argc; i++) {
    if (*argv[i] == '-') {
      /* number of sessions */
      if (strcmp(&argv[i][1], "s") == 0 && (argc > (i+1))) {
        session_size = (unsigned int) atoi(argv[++i]);
      }
      /* number of iterations */
      else if (strcmp(&argv[i][1], "i") == 0 && (argc > (i+1))) {
        iteration = (unsigned int) atoi(argv[++i]);
      }
      else if (strcmp(&argv[i][1], "l") == 0 && (argc > (i+1))) {
        loglevel = (unsigned int) atoi(argv[++i]);
      }
      else if (strcmp(&argv[i][1], "h") == 0 && (argc > (i+1))) {
        heartbeat_interval = (unsigned int) atoi(argv[++i]);
      }
      else if (strcmp(&argv[i][1], "m") == 0 && (argc > (i+1))) {
        monitor_interval = (unsigned int) atoi(argv[++i]);
      }
    }
  }

  if (session_size <= 0) session_size = 1;

  if (iteration <= 0) iteration = 10;

  /* print the usage */
  if (argc == 1) {
    printf("Usage: %s -s [number of sessions] -i [number of max iteration] -u [server url] -h [hearbeat interval] -m [monitor interval] -l [log level]\n", argv[0]);
  }

  printf("abort within 2 seconds or running with \n");

  /* print the current settings */
  printf("\tsessions:\t%d\n\tmax iteration:\t%d\n\tserver:\t\t%s\n\tloglevel:\t%d\n", 
         session_size, iteration, gateway_url, loglevel);

  /* count down */
  for (i = 2; i >= 1; i--) {
    printf("\r%d", i);
    fflush(stdout);
    sleep_random(1);
  }
  printf("\n");

  srand((unsigned int) time(NULL));

  settings = ccl_create_metadata();
  ccl_set_metadata_string(settings, CUSTOME_KEY, key);
  ccl_set_metadata_string(settings, GATEWAY_URL, gateway_url);
  ccl_set_metadata_int(settings, HEARTBEAT_INTERVAL, (int) heartbeat_interval);
  ccl_set_metadata_int(settings, ENABLE_LOGGING, 1);

  platform = ccl_create_metadata();
  ccl_set_metadata_string(platform, DEVICE_TYPE, "desktop");
  ccl_set_metadata_string(platform, DEVICE_BRAND, "dell");
  ccl_set_metadata_string(platform, DEVICE_MANUFACTURER, "dell");
  ccl_set_metadata_string(platform, DEVICE_MODEL, "xps");
  ccl_set_metadata_string(platform, DEVICE_VERSION, "1.0");
  ccl_set_metadata_string(platform, FRAMEWORK_NAME, "c");
  ccl_set_metadata_string(platform, FRAMEWORK_VERSION, "1.0");
  ccl_set_metadata_string(platform, OPERATING_SYSTEM_NAME, "linux");
  ccl_set_metadata_string(platform, OPERATING_SYSTEM_VERSION, "14.04");

  if (ccl_init(settings, platform, platform_interface) != 0) {
    fprintf(stderr, "ERROR: cclInitialize failed\n");
  }

  ccl_destroy_metadata(settings);
  ccl_destroy_metadata(platform);

  thread = (pthread_t *) malloc(sizeof(pthread_t*) * (unsigned long) session_size);

  for (i = 0; i < (int) session_size; i++) {
    pthread_create(&thread[i], NULL, worker, (void*) (long long) iteration);
  }

  sleep(1);

  for (i = 0; i < (int) session_size; i++) {
    pthread_join(thread[i], NULL);
  }

  pthread_detach(pthread_self());

  free(thread);

  ccl_deinit();

  return 0;
}

/* return a cdn name randomly */
static const char *get_cdn(void)
{
  static const char *cdn[] = {
    "AKAMAI",
    "AMAZON",
    "ATT",
    "BITGRAVITY",
    "BT",      
    "CDNETWORKS",
    "CDNVIDEO",  
    "CHINACACHE",
    "COMCAST",   
    "EDGECAST",  
    "HIGHWINDS", 
    "INHOUSE",   
    "INTERNAP",  
    "IPONLY",    
    "LEVEL3",    
    "LIMELIGHT", 
    "MICROSOFT", 
    "NGENIX",    
    "NICE",      
    "OCTOSHAPE", 
    "OTHER",     
    "QBRICK",    
    "SWARMCAST", 
    "TALKTALK",  
    "TELEFONICA",
    "TELENOR",   
    "VELOCIX",   
  };
  return cdn[rand()%(int)(sizeof(cdn)/sizeof(char*))];
}

static const char *get_message(void)
{
  static const char *msg[] = {
    "How many cares one loses when one decides not to be something but to be someone.",
    "Be who you are and say what you feel, because those who mind don't matter and those who matter don't mind.",
    "Imitation is suicide.",
    "Flatter yourself critically.",
    "Do what you feel in your heart to be right, for you'll be criticized anyway.",
    "Whenever you find yourself on the side of the majority, it is time to pause and reflect.",
    "I will not let anyone walk through my mind with their dirty feet.",
    "Better to write for yourself and have no public, than to write for the public and have no self."
    "We must not allow other people's limited perceptions to define us.",
    "Don't look for society to give you permission to be yourself.",
    "If things go wrong, don't go with them."
  };
  return msg[rand()%(int)(sizeof(msg)/sizeof(char*))];
}
