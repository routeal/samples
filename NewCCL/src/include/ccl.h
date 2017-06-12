#ifndef CCL_H
#define CCL_H

#ifdef __cplusplus
extern "C" {
#endif

/* settings */
#define CUSTOME_KEY                     "customer_key"
#define GATEWAY_URL                     "gateway_url"
#define HEARTBEAT_INTERVAL              "heartbeat_interval"
#define ENABLE_LOGGING                  "enable_logging"

/* platform metadata */
#define DEVICE_TYPE                     "device_type"
#define DEVICE_BRAND                    "device_brand"
#define DEVICE_MANUFACTURER             "device_manufacturer"
#define DEVICE_MODEL                    "device_model"
#define DEVICE_VERSION                  "device_version"
#define FRAMEWORK_NAME                  "framework_name"
#define FRAMEWORK_VERSION               "framework_version"
#define OPERATING_SYSTEM_NAME           "operating_system_name"
#define OPERATING_SYSTEM_VERSION        "operating_system_version"

/* content info */
#define PLAYER_NAME                     "player_name"
#define ASSET_NAME                      "asset_name"
#define VIEWER_ID                       "viewer_id"
#define CDN_NAME                        "cdn_name"
#define MEDIA_RESOURCE                  "resource"
#define MEDIA_URL                       "url"
#define MEDIA_DURATION                  "duration"
#define MEDIA_BITRATE                   "bitrate"
#define MEDIA_IS_LIVE                   "is_live"
#define MEDIA_TAGS                      "tags"

typedef enum {
  CCL_ERROR                 = -1, /* general error */
  CCL_SUCCESS               = 0, /* success */
  CCL_INVALID_PARAMETER     = 1,
  CCL_SESSION_NOT_AVAILABLE = 2,
  CCL_SESSION_IS_GLOBAL     = 3,
  CCL_NOT_ENOUGH_MEMORY     = 4,
  CCL_TOO_LONG_STRING       = 5,
} ccl_error_state;

typedef enum {
  CCL_STOPPED               = 1,
  CCL_PLAYING               = 3,
  CCL_BUFFERING             = 6,
  CCL_PAUSED                = 12,
  CCL_NOTMONITORED          = 98,
  CCL_UNKNOWN               = 100
} ccl_playback_state;

typedef struct ccl_session ccl_session;

typedef struct ccl_player {
  int  (*get_playhead_time) (void *data);
  int  (*get_buffered_time) (void *data);
  int  (*get_framerate)     (void *data);
} ccl_player;

typedef void (*ccl_timer_callback) (void *data);

typedef struct ccl_platform_interface {
  /** prints the log to console */
  int (*print_log) (const char *log, unsigned int logsiz);

  /** saves the config */
  int (*save_config) (const char *buf, unsigned int bufsiz);

  /** loads the config */
  int (*load_config) (char *buf, unsigned int bufsiz);

  /** sends the heartbeat to the Conviva server */
  int (*send_heartbeat) (const char *url, const char *content_type,
                         const char *heartbeat, unsigned int heartbeat_size);

  /** creates and starts a timer */
  void * (*create_timer) (ccl_timer_callback func, void *data,
                          unsigned int initial_time, unsigned int interval);

  /** destroys the timer */
  void (*destroy_timer) (void *timer_handle);

} ccl_platform_interface;

typedef void ccl_tag;

typedef void ccl_metadata;

extern ccl_tag *ccl_create_tag(void);

extern void ccl_destroy_tag(ccl_tag *tags);

extern void ccl_set_tag(ccl_tag *tags, const char *key, const char *value);

extern ccl_metadata *ccl_create_metadata(void);

extern void ccl_destroy_metadata(ccl_metadata *m);

extern void ccl_set_metadata_string(ccl_metadata *m, const char *key, const char *value);

extern void ccl_set_metadata_int(ccl_metadata *m, const char *key, int value);

extern void ccl_set_metadata_tag(ccl_metadata *m, const ccl_tag *tags);

extern const char *ccl_get_metadata_string(ccl_metadata *m, const char *key);

extern int ccl_get_metadata_int(ccl_metadata *m, const char *key);

extern ccl_tag *ccl_get_metadata_tag(ccl_metadata *m);

extern int ccl_init(const ccl_metadata *settings, const ccl_metadata *platform, const ccl_platform_interface *pif);

extern int ccl_deinit(void);

extern int ccl_send_event(const char *name, const ccl_tag *tags);

extern int ccl_parse_hearbeat(const char *response, unsigned int size);

extern ccl_session* ccl_session_create(const ccl_metadata *content, int *error);

extern int ccl_session_destroy(ccl_session *s);

extern int ccl_session_attach(ccl_session *s, const ccl_player *p, void *data);

extern int ccl_session_detach(ccl_session *s);

extern int ccl_session_suspend(ccl_session *s);

extern int ccl_session_resume(ccl_session *s);

extern int ccl_session_end(ccl_session *s);

extern int ccl_session_error(ccl_session *s, const char *error, int fatal);

extern int ccl_session_log(ccl_session *s, const char *log);

extern int ccl_session_update_playback_state(ccl_session *s, int playback_state);

extern int ccl_session_update_duration(ccl_session *s, int duration);

extern int ccl_session_update_framerate(ccl_session *s, int framerate);

extern int ccl_session_update_bitrate(ccl_session *s, int bitrate);

extern int ccl_session_update_cdn(ccl_session *s, const char *cdn);

extern int ccl_session_update_resource(ccl_session *s, const char *url);

extern int ccl_session_send_event(ccl_session *s, const char *name, const ccl_tag *tags);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif
