#ifndef SESSION_H
#define SESSION_H

#include "jvalue.h"

struct ccl_session {
  unsigned int id;
  unsigned int heartbeat_number;
  unsigned int event_number;
  unsigned int heartbeat_interval;

  int do_logging;
  int being_destroyed;

  /* session flag */
  int flag;

  int playback_state;

  unsigned long long int start_time;

  unsigned long long int suspended_time;

  void *heartbeat_timer_handle;

  /* content info */
  jvalue *content;

  jvalue *current_events; /* points either events1 or events2 */
  jvalue *events1;
  jvalue *events2;

  jvalue *logs1;
  jvalue *logs2;
  jvalue *current_logs;

  /* array object */
  jvalue *afps;

  /* attached player */
  const ccl_player *player;
  void *player_data;
};

extern ccl_session *gsession;

extern int ccl_session_get_state(ccl_session *s);

extern ccl_session* ccl_session_get(int id);

extern jvalue *content_info_metadata(const ccl_metadata *content);

#endif
