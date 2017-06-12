#include "ccl.h"
#include "ccl_int.h"
#include "session.h"
#include "port.h"
#include "protocol.h"
#include <stdio.h>

#define SESSION_INITIAL_DELAY (2 * 1000)

/** settings to be set at initialization */
jvalue *jsettings;
/** platform metadata */
jvalue *jmetadata;

/** global session */
ccl_session *gsession;

/** protocol handler */
protocol *current_protocol;

/** platform interface */
static ccl_platform_interface sinterface;
ccl_platform_interface *pinterface = &sinterface;

static char the_client_id_tmp[128];
char *the_client_id = &the_client_id_tmp[0];
static char tmp_client_id_tmp[128];
char *tmp_client_id = &tmp_client_id_tmp[0];

unsigned int the_hb_interval;

/** session container */
static jvalue *jsessions;

static void ccl_session_add(ccl_session *s);
static void ccl_session_start(ccl_session *s);
static void ccl_session_send_heartbeat(ccl_session *s);
static void on_heartbeat_timer(void *arg);

int ccl_init(const ccl_metadata *settings, const ccl_metadata *platform, const ccl_platform_interface *pif)
{
  if (settings == 0 || platform == 0 || pif == 0) return CCL_INVALID_PARAMETER;

  current_protocol = protocol_17;

  jsettings = settings_metadata(settings);

  jmetadata = platform_metadata(platform);

  pinterface->save_config = pif->save_config;
  pinterface->load_config = pif->load_config;
  pinterface->send_heartbeat = pif->send_heartbeat;
  pinterface->print_log = pif->print_log;
  pinterface->create_timer = pif->create_timer;
  pinterface->destroy_timer = pif->destroy_timer;

  /* reading the persistent data */
  load_persistent_data();

  jsessions = jobject_create();

  /* global session */
  gsession = (ccl_session *) port_malloc(sizeof(ccl_session));
  gsession->heartbeat_interval = (unsigned int) jobject_get_int(jsettings, HEARTBEAT_INTERVAL, 0);
  gsession->id = port_rand();
  gsession->flag = CCL_CAPABILITY_GLOBAL;
  gsession->start_time = port_current_time();
  gsession->events1 = jarray_create();
  gsession->events2 = jarray_create();
  gsession->current_events = gsession->events1;
  gsession->logs1 = jarray_create();
  gsession->logs2 = jarray_create();
  gsession->current_logs = gsession->logs1;

  ccl_session_add(gsession);
  ccl_session_start(gsession);

  return 0;
}

int ccl_deinit(void)
{
  ccl_session_destroy(gsession);

  jvalue_destroy(jsessions);
  jvalue_destroy(jsettings);
  jvalue_destroy(jmetadata);
  return 0;
}

int ccl_send_event(const char *name, const ccl_tag *t)
{
  return ccl_session_send_event(gsession, name, t);
}

/* this may be executed in the different thread */
int ccl_parse_hearbeat(const char *response, unsigned int size)
{
  return current_protocol->parse_response(response, size);
}

int load_persistent_data(void)
{
  char buf[1024];
  if (pinterface->load_config(buf, 1024) == 0) {
    jvalue *config = jvalue_read(buf, 0);
    const char *clid = jobject_get_string(config, CLIENT_ID, 0);
    if (clid) {
      port_strncpy(the_client_id, clid, 128);
      port_strncpy(tmp_client_id, clid, 128);
    }
    jvalue_destroy(config);
  }
  return 0;
}

int save_persistent_data(void)
{
  jvalue *config = jobject_create();
  jobject_put_string(config, CLIENT_ID, the_client_id);
  unsigned int len = 0;
  char *str = jvalue_write(config, &len);
  int status = pinterface->save_config(str, len);
  port_free(str);
  jvalue_destroy(config);
  return status;
}

ccl_session* ccl_session_create(const ccl_metadata *content, int *error)
{
  if (content == 0) {
    if (error) *error = CCL_INVALID_PARAMETER;
    return 0;
  }

  ccl_session *s = (ccl_session *) port_malloc(sizeof(ccl_session));

  if (s == 0) {
    if (error) *error = CCL_NOT_ENOUGH_MEMORY;
    return 0;
  }

  s->flag = CCL_CAPABILITY_VIDEO|CCL_CAPABILITY_QUALITY_METRICS|CCL_CAPABILITY_BITRATE_METRICS;

  s->id = port_rand();

  s->content = content_info_metadata(content);

  s->heartbeat_interval = (unsigned int) jobject_get_int(jsettings, HEARTBEAT_INTERVAL, 0);

  s->start_time = port_current_time();

  s->events1 = jarray_create();
  s->events2 = jarray_create();
  s->current_events = s->events1;

  s->logs1 = jarray_create();
  s->logs2 = jarray_create();
  s->current_logs = s->logs1;

  s->afps = jarray_create();

  ccl_session_add(s);

  ccl_session_start(s);

  return s;
}

void ccl_session_start(ccl_session *s)
{
  /* cancel the timer if any */
  if (s->heartbeat_timer_handle && pinterface->destroy_timer) {
    pinterface->destroy_timer(s->heartbeat_timer_handle);
  }

  if (pinterface->create_timer) {
    unsigned int interval = (unsigned int) jobject_get_int(jsettings, HEARTBEAT_INTERVAL, 0);

    //printf("ccl_session_start: HB Interval=%d s=%p\n", interval, (void*) s);

    s->heartbeat_timer_handle = pinterface->create_timer(
        on_heartbeat_timer, s, SESSION_INITIAL_DELAY, interval);
  }
}

static void ccl_session_stop(ccl_session *s)
{
  /* cancel the timer if any */
  if (s->heartbeat_timer_handle) {
    void *tmp = s->heartbeat_timer_handle;
    s->heartbeat_timer_handle = 0;
    pinterface->destroy_timer(tmp);
  }
}

#if 0
static void ccl_session_destroy_wrapper(void *d)
{
  ccl_session *s = (ccl_session *) d;
  ccl_session_destroy(s);
}
#endif

void ccl_session_add(ccl_session *s)
{
  char buf[64];
  port_inttostr(buf, 64, s->id);
  //jobject_put_user(jsessions, buf, s, 0, ccl_session_destroy_wrapper);
  jobject_put_user(jsessions, buf, s, 0, 0);
}

static void ccl_session_remove(ccl_session *s)
{
  printf("ccl_session_remove id=%d\n", s->id);
  char buf[64];
  port_inttostr(buf, 64, s->id);
  jobject_remove(jsessions, buf);
}

ccl_session* ccl_session_get(int id)
{
  char buf[64];
  port_inttostr(buf, 64, id);
  int error;
  juser *user = (juser *) jobject_get_user(jsessions, buf, &error);
  if (user) return (ccl_session *) user->value;
  return 0;
}

int ccl_session_end(ccl_session *s)
{
  ccl_session_stop(s);
  if (s->flag != CCL_CAPABILITY_GLOBAL) {
    jvalue *evt = current_protocol->compose_session_end_event(s);
    jarray_put(s->current_events, evt);
    ccl_session_send_heartbeat(s);
    return 0;
  }
  return CCL_SESSION_IS_GLOBAL;
}

int ccl_session_destroy(ccl_session *s)
{
  s->being_destroyed = 1;

  printf("ccl_session_destroy id=%d\n", s->id);

  //  ccl_session_stop(s);

  /*
  if (s->flag != CCL_CAPABILITY_GLOBAL) {
    jvalue *evt = current_protocol->compose_session_end_event(s);
    jarray_put(s->current_events, evt);
    ccl_session_send_heartbeat(s);
  }
  */

  ccl_session_remove(s);
  jvalue_destroy(s->content);
  jvalue_destroy(s->logs1);
  jvalue_destroy(s->logs2);
  jvalue_destroy(s->events1);
  jvalue_destroy(s->events2);
  jvalue_destroy(s->afps);
  port_free(s);
  return 0;
}

int ccl_session_attach(ccl_session *s, const ccl_player *p, void *data)
{
  if (s->player) return -1;
  printf("ccl_session\n");
  s->player = p;
  s->player_data = data;
  ccl_session_update_playback_state(s, CCL_UNKNOWN);
  return 0;
}

int ccl_session_detach(ccl_session *s)
{
  if (s->player == 0) return -1;
  printf("ccl_session_detach\n");
  ccl_session_update_playback_state(s, CCL_NOTMONITORED);
  s->player = 0;
  s->player_data = 0;
  return 0;
}

int ccl_session_suspend(ccl_session *s)
{
  if (s->flag ==  CCL_CAPABILITY_GLOBAL) return -1;
  if (s->suspended_time > 0) return -1;
  s->suspended_time = port_current_time();
  jvalue *evt = current_protocol->compose_pausejoin_change_event(s, 1, 0);
  jarray_put(s->current_events, evt);
  return 0;
}

int ccl_session_resume(ccl_session *s)
{
  if (s->flag ==  CCL_CAPABILITY_GLOBAL) return -1;
  if (s->suspended_time == 0) return -1;
  s->suspended_time = 0;
  jvalue *evt = current_protocol->compose_pausejoin_change_event(s, 0, 1);
  jarray_put(s->current_events, evt);
  return 0;
}

int ccl_session_error(ccl_session *s, const char *error, int fatal)
{
  if (s->flag ==  CCL_CAPABILITY_GLOBAL) return -1;
  if (s->player == 0) return -1;
  jvalue *evt = current_protocol->compose_error_event(s, error, fatal);
  jarray_put(s->current_events, evt);
  return 0;
}

int ccl_session_log(ccl_session *s, const char *log)
{
  jarray_put_string(s->current_logs, log);
  return 0;
}

int ccl_session_update_playback_state(ccl_session *s, int playback_state)
{
  if (s->player == 0) return -1;
  int old_playback_state = ccl_session_get_state(s);
  if (playback_state == old_playback_state) return -1;
  jvalue *evt = current_protocol->compose_playstate_change_event(
      s, playback_state, ccl_session_get_state(s));
  jarray_put(s->current_events, evt);
  s->playback_state = playback_state;
  return 0;
}

int ccl_session_update_duration(ccl_session *s, int duration)
{
  if (s->player == 0) return -1;
  jobject_put_int(s->content, "duration", duration);
  return 0;
}

int ccl_session_update_framerate(ccl_session *s, int framerate)
{
  if (s->player == 0) return -1;
  jobject_put_int(s->content, "framerate", framerate);
  return 0;
}

int ccl_session_update_bitrate(ccl_session *s, int bitrate)
{
  if (s->player == 0) return -1;
  int current_bitrate = jobject_get_int(s->content, "bitrate", 0);
  jvalue *evt = current_protocol->compose_bitrate_change_event(s, bitrate, current_bitrate);
  jarray_put(s->current_events, evt);
  jobject_put_int(s->content, "bitrate", bitrate);
  return 0;
}

int ccl_session_update_cdn(ccl_session *s, const char *cdn)
{
  if (s->player == 0) return -1;
  const char *current_cdn = jobject_get_string(s->content, "cdn_name", 0);
  jvalue *evt = current_protocol->compose_cdnname_change_event(s, cdn, current_cdn);
  jarray_put(s->current_events, evt);
  jobject_put_string(s->content, "cdn_name", cdn);
  return 0;
}

int ccl_session_update_resource(ccl_session *s, const char *url)
{
  if (s->player == 0) return -1;
  const char *current_url = jobject_get_string(s->content, "url", 0);
  jvalue *evt = current_protocol->compose_resource_change_event(s, url, current_url);
  jarray_put(s->current_events, evt);
  jobject_put_string(s->content, "url", url);
  return 0;
}

int ccl_session_send_event(ccl_session *s, const char *name, const ccl_tag *tags)
{
  if (s == 0 || name == 0 || tags == 0) return CCL_INVALID_PARAMETER;

  const jvalue *jtags = (const jvalue *) tags;

  jvalue *event = jvalue_clone(jtags);

  if (event == 0) return CCL_NOT_ENOUGH_MEMORY;

  jvalue *evt = current_protocol->compose_custom_event(s, name, event);

  if (evt == 0) return CCL_NOT_ENOUGH_MEMORY;

  jarray_put(s->current_events, evt);

  return 0;
}

int ccl_session_get_state(ccl_session *s)
{
  if (s->suspended_time > 0) return CCL_NOTMONITORED;

  return s->playback_state;
}

static void ccl_session_send_heartbeat(ccl_session *s)
{
  unsigned int length = 0;
  char *request = current_protocol->compose_request(s, &length);

  const char *url = jobject_get_string(jsettings, GATEWAY_URL, 0);

  if (pinterface->send_heartbeat(url, "Content-Type: application/json", request, length)) {
    /* error */
    //pinterface->print_log("failed to send a heartbeat");
  }

  port_free(request);
}

static void on_heartbeat_timer(void *arg)
{
  ccl_session *s = (ccl_session *) arg;

  if (s == 0) {
    //printf("on_heartbeat_timer: null session\n");
    return;
  }

  if (s->being_destroyed) return;

  if (s->heartbeat_timer_handle == 0) return;

  if (jarray_size(s->current_events) == 0) {
    if (s->flag == CCL_CAPABILITY_GLOBAL) {
      //printf("on_heartbeat_timer: global, no event\n");
      return;
    }
  }

  ccl_session_send_heartbeat(s);

  /* update the interval if necessary */
  if (s->heartbeat_interval != the_hb_interval) {
    s->heartbeat_interval = the_hb_interval;
    ccl_session_start(s);
  }

  /* save the clid */
  if (port_strcmp(the_client_id, tmp_client_id)) {
    port_strncpy(the_client_id, tmp_client_id, 128);
    save_persistent_data();
  }
}
