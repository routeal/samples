#include "ccl.h"
#include "ccl_int.h"
#include "session.h"
#include "protocol.h"
#include "port.h"

static void compose_mandatory_fields(jvalue *cws, ccl_session *s)
{
  /* protocol version */
  jobject_put_string(cws, "pver", current_protocol->version);

  /* type */
  jobject_put_string(cws, "t", "CwsSessionHb");

  /* customerid */
  jobject_put_string(cws, "cid", jobject_get_string(jsettings, "customer_key", 0));

  /* cliendid */
  if (*the_client_id) {
    jobject_put_string(cws, "clid", the_client_id);
  }

  /* instanceid */
  jobject_put_int(cws, "iid", jobject_get_int(jsettings, "player_id", 0));

  /* sessionid */
  jobject_put_int(cws, "sid", (int) s->id);

  /* sessiontimems */
  int session_time = (int) (port_current_time() - s->start_time);
  jobject_put_int(cws, "st", session_time);

  /* seqnumber */
  jobject_put_int(cws, "seq", (int) s->heartbeat_number++);

  /* clienttimesec */
  double client_time = (double) (port_current_time() / 1000.0);
  jobject_put_real(cws, "cts", client_time);

  /* session flag */
  jobject_put_int(cws, "sf", s->flag);

  /* clientversion */
  jobject_put_string(cws, "clv", CCL_VERSION);
}

static void compose_optional_fields(jvalue *cws, ccl_session *s)
{
  /* events */
  if (s->current_events == s->events1) {
    s->current_events = s->events2;
    jobject_put_array(cws, "evs", s->events1);
    s->events1 = jarray_create();
  } else {
    s->current_events = s->events1;
    jobject_put_array(cws, "evs", s->events2);
    s->events2 = jarray_create();
  }

  /* tags */
  jvalue *tags = (jvalue *) jobject_get_object(s->content, "tags", 0);
  if (tags) {
    jvalue *tags_clone = jvalue_clone(tags);
    jobject_put_object(cws, "tags", tags_clone);
  }

  /* viewerid */
  jobject_put_string(cws, "vid", jobject_get_string(s->content, "viewer_id", 0));

  /* framework */
  jobject_put_string(cws, "fw", jobject_get_string(jmetadata, "framework_name", 0));

  /* frameworkversion */
  jobject_put_string(cws, "fwv", jobject_get_string(jmetadata, "framework_version", 0));

  /* platform */
  jobject_put_string(cws, "pt", CCL_PLATFORM);

  /* platform metadata */
  jvalue *metadata_clone = jvalue_clone(jmetadata);
  jobject_put_object(cws, "pm", metadata_clone);

  /* TODO: logs */
  /* lg */
  /* jobject_put_string(cws, "lg", "OTHER"); */

  /* ct */
  jobject_put_string(cws, "ct", "OTHER");

  /* sdk */
  jobject_put_bool(cws, "sdk", 1);
}

static void compose_video_session_required_fields(jvalue *cws, ccl_session *s)
{
  if (!(s->flag & CCL_CAPABILITY_VIDEO)) return;

  /* aseet name */
  jobject_put_string(cws, "an", jobject_get_string(s->content, "asset_name", 0));

  /* playing state */
  jobject_put_int(cws, "ps", ccl_session_get_state(s));

  /* pausejoin */
  jobject_put_bool(cws, "pj", s->suspended_time ? 1 : 0);
}

static void compose_video_session_optional_fields(jvalue *cws, ccl_session *s)
{
  if (!(s->flag & CCL_CAPABILITY_VIDEO)) return;

  /* resource */
  jobject_put_string(cws, "rs", jobject_get_string(s->content, "resource", 0));

  /* cdnname */
  jobject_put_string(cws, "cdn", jobject_get_string(s->content, "cdn_name", 0));

  /* contentlengthsec */
  jobject_put_int(cws, "cl", jobject_get_int(s->content, "duration", 0));

  /* playername */
  jobject_put_string(cws, "pn", jobject_get_string(s->content, "player_name", 0));

  /* bitratekbps */
  jobject_put_int(cws, "br", jobject_get_int(s->content, "bitrate", 0));

  /* encodedfps */
  jobject_put_int(cws, "efps", jobject_get_int(s->content, "framerate", 0));

  /* averagefps */
#if 1
  if (ccl_session_get_state(s) == CCL_PLAYING) {
    unsigned int i;
    int afps = 0;
    jarray_put_int(s->afps, s->player->get_framerate(s->player_data));
    for (i = 0; i < jarray_size(s->afps); i++) {
      jint *v = (jint *) jarray_get(s->afps, i, 0);
      afps += v->value;
    }
    afps /= i;
    jobject_put_int(cws, "afps", afps);
  }
#endif

  /* streamurl */
  jobject_put_string(cws, "url", jobject_get_string(s->content, "url", 0));

  /* islive */
  int is_live = jobject_get_int(s->content, "is_live", 0);
  if (is_live >= 0) {
    jobject_put_bool(cws, "lv", is_live);
  }
}

static char *compose_request(ccl_session *s, unsigned int *length)
{
  jvalue *cws = jobject_create();

  /* FIXME: returns an error??? */
  if (cws == 0) return 0;

  compose_mandatory_fields(cws, s);
  compose_optional_fields(cws, s);
  compose_video_session_required_fields(cws, s);
  compose_video_session_optional_fields(cws, s);

  /* convert to json string */
  char *str = jvalue_write(cws, length);

  jvalue_destroy(cws);

  return str;
}

static int parse_response(const char *response, unsigned int response_length)
{
  if (response == 0 || response_length == 0) return CCL_INVALID_PARAMETER;

  /* verify the buffer content */
  if (response_length > MAX_HEARBEAT_RESPONCE_SIZE) return CCL_TOO_LONG_STRING;

  int error = 0;

  /* verfity that the string is null terminated */
  char *buf = port_strndup(response, response_length + 1);
  if (buf == 0) return CCL_NOT_ENOUGH_MEMORY;
  buf[response_length] = 0;

  jvalue *v = jvalue_read(buf, 0);

  port_free(buf);

  if (v == 0) return CCL_NOT_ENOUGH_MEMORY;

  const char *t = jobject_get_string(v, "t", &error);
  if (t == 0 || port_strncasecmp(t, "CwsServerResponse", 17) != 0) {
    error = -1;
  }

  const char *err = jobject_get_string(v, "err", &error);
  if (err == 0 || port_strncasecmp(err, "err", 3) != 0) {
    error = -1;
  }

  int session_id = jobject_get_int(v, "sid", &error);
  ccl_session* s = ccl_session_get(session_id);
  if (s == 0) error = -1;

  const char *client_id = jobject_get_string(v, "clid", &error);
  if (client_id) {
    if (port_strcmp(client_id, tmp_client_id)) {
      port_strncpy(tmp_client_id, client_id, 128);
    }
  }

  int slg = jobject_get_bool(v, "slg", &error);
  if (error == 0) {
    if (s) s->do_logging = slg;
  }

  unsigned int hbi = (unsigned int) jobject_get_int(v, "hbi", &error);
  if (error == 0) {
    if (hbi != the_hb_interval) {
      the_hb_interval = hbi;
    }
  }

  const char *gw = jobject_get_string(v, "gw", &error);
  if (gw) {
    jobject_put_string(jsettings, GATEWAY_URL, gw);
  }

  jvalue_destroy(v);

  return error;
}

static jvalue *compose_event(ccl_session *s, const char* type)
{
  jvalue *cws = jobject_create();
  int elapsed = (int) (port_current_time() - s->start_time);
  jobject_put_string(cws, "t", type);
  jobject_put_int(cws, "st", elapsed);
  jobject_put_int(cws, "seq", (int) s->event_number++);
  return cws;
}

static jvalue *compose_int_change_event(ccl_session *s, const char* key, int new_value, int old_value)
{
  jvalue *cws = compose_event(s, "CwsStateChangeEvent");
  if (old_value >= 0) {
    jvalue *oldvalue = jobject_create();
    jobject_put_int(oldvalue, key, old_value);
    jobject_put_object(cws, "old", oldvalue);
  }
  jvalue *newvalue = jobject_create();
  jobject_put_int(newvalue, key, new_value);
  jobject_put_object(cws, "new", newvalue);
  return cws;
}

static jvalue *compose_string_change_event(ccl_session *s, const char* key, const char *new_value, const char *old_value)
{
  jvalue *cws = compose_event(s, "CwsStateChangeEvent");
  if (old_value) {
    jvalue *oldvalue = jobject_create();
    jobject_put_string(oldvalue, key, old_value);
    jobject_put_object(cws, "old", oldvalue);
  }
  jvalue *newvalue = jobject_create();
  jobject_put_string(newvalue, key, new_value);
  jobject_put_object(cws, "new", newvalue);
  return cws;
}

static jvalue *compose_object_change_event(ccl_session *s, const char* key, jvalue *new_value, jvalue *old_value)
{
  jvalue *cws = compose_event(s, "CwsStateChangeEvent");
  if (old_value) {
    jvalue *oldvalue = jobject_create();
    jobject_put_object(oldvalue, key, old_value);
    jobject_put_object(cws, "old", oldvalue);
  }
  jvalue *newvalue = jobject_create();
  jobject_put_object(newvalue, key, new_value);
  jobject_put_object(cws, "new", newvalue);
  return cws;
}

static jvalue *compose_playstate_change_event(ccl_session *s, int new_state, int old_state)
{
  return compose_int_change_event(s, "ps", new_state, old_state);
}

static jvalue *compose_bitrate_change_event(ccl_session *s, int newbitrate, int oldbitrate)
{
  return compose_int_change_event(s, "br", newbitrate, oldbitrate);
}

static jvalue *compose_pausejoin_change_event(ccl_session *s, int newpausejoin, int oldpausejoin)
{
  return compose_int_change_event(s, "pj", newpausejoin, oldpausejoin);
}

static jvalue *compose_resource_change_event(ccl_session *s, const char *newresource, const char *oldresource)
{
  return compose_string_change_event(s, "rs", newresource, oldresource);
}

static jvalue *compose_cdnname_change_event(ccl_session *s, const char *newcdn, const char *oldcdn)
{
  return compose_string_change_event(s, "cdn", newcdn, oldcdn);
}

static jvalue *compose_assetname_change_event(ccl_session *s, const char *newname, const char *oldname)
{
  return compose_string_change_event(s, "an", newname, oldname);
}

static jvalue *compose_tags_change_event(ccl_session *s, jvalue *newtags, jvalue *oldtags)
{
  return compose_object_change_event(s, "tags", newtags, oldtags);
}

static jvalue *compose_error_event(ccl_session *s, const char *error, int is_fatal)
{
  if (error == 0) return 0;
  jvalue *cws = compose_event(s, "CwsErrorEvent");
  jobject_put_bool(cws, "ft", is_fatal);
  jobject_put_string(cws, "err", error);
  return cws;
}

static jvalue *compose_custom_event(ccl_session *s, const char *name, jvalue *event)
{
  jvalue *cws = compose_event(s, "CwsCustomEvent");
  jobject_put_string(cws, "name",  name);
  jobject_put_object(cws, "attr", event);
  return cws;
}

static jvalue *compose_session_end_event(ccl_session *s)
{
  if (s->flag ==  CCL_CAPABILITY_GLOBAL) return 0;
  return compose_event(s, "CwsSessionEndEvent");
}

static protocol s_protocol_17 = {
  "1.7",
  compose_request,
  parse_response,
  compose_playstate_change_event,
  compose_bitrate_change_event,
  compose_pausejoin_change_event,
  compose_resource_change_event,
  compose_cdnname_change_event,
  compose_assetname_change_event,
  compose_tags_change_event,
  compose_error_event,
  compose_custom_event,
  compose_session_end_event,
};

protocol *protocol_17 = &s_protocol_17;
