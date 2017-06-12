#include "ccl.h"
#include "ccl_int.h"
#include "session.h"
#include "port.h"

ccl_tag *ccl_create_tag(void)
{
  return (ccl_tag *) jobject_create();
}

void ccl_destroy_tag(ccl_tag *t)
{
  jvalue_destroy((jvalue *) t);
}

void ccl_set_tag(ccl_tag *t, const char *key, const char *value)
{
  jvalue *v = (jvalue *) t;
  jobject_put_string(v, key, value);
}

ccl_metadata *ccl_create_metadata(void)
{
  return (ccl_metadata *) jobject_create();
}

void ccl_destroy_metadata(ccl_metadata *m)
{
  jvalue_destroy((jvalue *) m);
}

void ccl_set_metadata_string(ccl_metadata *m, const char *key, const char *value)
{
  jvalue *v = (jvalue *) m;
  jobject_put_string(v, key, value);
}

void ccl_set_metadata_int(ccl_metadata *m, const char *key, int value)
{
  jvalue *v = (jvalue *) m;
  jobject_put_int(v, key, value);
}

void ccl_set_metadata_tag(ccl_metadata *m, const ccl_tag *t)
{
  const jvalue *tt = (const jvalue *) t;
  if (tt == 0) return;
  jvalue *cc = jvalue_clone(tt);
  jvalue *mm = (jvalue *) m;
  jobject_put_object(mm, "tags", cc);
}

const char *ccl_get_metadata_string(ccl_metadata *m, const char *key)
{
  if (m == 0 || key == 0) return 0;
  jvalue *v = (jvalue *) m;
  return jobject_get_string(v, key, 0);
}

int ccl_get_metadata_int(ccl_metadata *m, const char *key)
{
  if (m == 0 || key == 0) return 0;
  jvalue *v = (jvalue *) m;
  return jobject_get_int(v, key, 0);
}

ccl_tag *ccl_get_metadata_tag(ccl_metadata *m)
{
  if (m == 0) return 0;
  jvalue *v = (jvalue *) m;
  return (ccl_tag *) jobject_get_object(v, "tags", 0);
}

jvalue *settings_metadata(const ccl_metadata *settings)
{
  const jvalue *s = (const jvalue *) settings;
  if (s == 0) return 0;
  jvalue *v = jobject_create();
  if (v == 0) return 0;
  const char *custome_key = jobject_get_string(s, CUSTOME_KEY, 0);
  if (custome_key) {
    jobject_put_string(v, CUSTOME_KEY, custome_key);
  }
  const char *gateway_url = jobject_get_string(s, GATEWAY_URL, 0);
  if (gateway_url) {
    jobject_put_string(v, GATEWAY_URL, gateway_url);
  }
  int heartbeat_interval = jobject_get_int(s, HEARTBEAT_INTERVAL, 0);
  if (heartbeat_interval) {
    jobject_put_int(v, HEARTBEAT_INTERVAL, heartbeat_interval);
  } else {
    jobject_put_int(v, HEARTBEAT_INTERVAL, DEFAULT_HEARTBEAT_INTERVAL);
  }
  const char *enable_logging = jobject_get_string(s, ENABLE_LOGGING, 0);
  if (enable_logging) {
    jobject_put_string(v, ENABLE_LOGGING, enable_logging);
  }
  jobject_put_int(v, "player_id", (int) port_rand());
  return v;
}

jvalue *platform_metadata(const ccl_metadata *platform)
{
  const jvalue *p = (const jvalue *) platform;
  if (p == 0) return 0;
  jvalue *v = jobject_create();
  if (v == 0) return 0;
  jobject_put_string(v, "sch", CCL_PLATFORM_METADATA_SCHEMA);
  const char *dvt = jobject_get_string(p, DEVICE_TYPE, 0);
  if (dvt) {
    jobject_put_string(v, "dvt", dvt);
  }
  const char *dvb = jobject_get_string(p, DEVICE_BRAND, 0);
  if (dvb) {
    jobject_put_string(v, "dvb", dvb);
  }
  const char *dvma = jobject_get_string(p, DEVICE_MANUFACTURER, 0);
  if (dvma) {
    jobject_put_string(v, "dvma", dvma);
  }
  const char *dvm = jobject_get_string(p, DEVICE_MODEL, 0);
  if (dvm) {
    jobject_put_string(v, "dvm", dvm);
  }
  const char *dvv = jobject_get_string(p, DEVICE_VERSION, 0);
  if (dvv) {
    jobject_put_string(v, "dvv", dvv);
  }
  const char *fw = jobject_get_string(p, FRAMEWORK_NAME, 0);
  if (fw) {
    jobject_put_string(v, "fw", fw);
  }
  const char *fwv = jobject_get_string(p, FRAMEWORK_VERSION, 0);
  if (fwv) {
    jobject_put_string(v, "fwv", fwv);
  }
  const char *os = jobject_get_string(p, OPERATING_SYSTEM_NAME, 0);
  if (os) {
    jobject_put_string(v, "os", os);
  }
  const char *osv = jobject_get_string(p, OPERATING_SYSTEM_VERSION, 0);
  if (osv) {
    jobject_put_string(v, "osv", osv);
  }
  return v;
}

jvalue *content_info_metadata(const ccl_metadata *content)
{
  return jvalue_clone((const jvalue *) content);
}
