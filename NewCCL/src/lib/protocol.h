typedef struct protocol protocol;

typedef enum {
    CCL_CAPABILITY_GLOBAL           = 0x0000,
    CCL_CAPABILITY_VIDEO            = 0x0001,
    CCL_CAPABILITY_QUALITY_METRICS  = 0x0002,
    CCL_CAPABILITY_BITRATE_METRICS  = 0x0004
} capability;

struct protocol {
  char *version;
  char * (*compose_request) (ccl_session *s, unsigned int *length);
  int (*parse_response) (const char *response, unsigned int response_length);
  jvalue *(*compose_playstate_change_event)(ccl_session *s, int new_state, int old_state);
  jvalue *(*compose_bitrate_change_event)(ccl_session *s, int newbitrate, int oldbitrate);
  jvalue *(*compose_pausejoin_change_event)(ccl_session *s, int newpausejoin, int oldpausejoin);
  jvalue *(*compose_resource_change_event)(ccl_session *s, const char *newresource, const char *oldresource);
  jvalue *(*compose_cdnname_change_event)(ccl_session *s, const char *newcdn, const char *oldcdn);
  jvalue *(*compose_assetname_change_event)(ccl_session *s, const char *newname, const char *oldname);
  jvalue *(*compose_tags_change_event)(ccl_session *s, jvalue *newtags, jvalue *oldtags);
  jvalue *(*compose_error_event)(ccl_session *s, const char *error, int is_fatal);
  jvalue *(*compose_custom_event)(ccl_session *s, const char *name, jvalue *event);
  jvalue *(*compose_session_end_event)(ccl_session *s);
};

extern protocol *protocol_17;

extern protocol *current_protocol;

