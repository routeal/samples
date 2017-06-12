#ifndef CCL_INT_H
#define CCL_INT_H

#include "jvalue.h"

#define CLIENT_ID                       "clid"

#define PLAYER_ID                       "player_id"

#define CCL_VERSION                     "2.116.0.31425"

#define CCL_PLATFORM                    "C"

#define CCL_PLATFORM_METADATA_SCHEMA    "gen1"

#define DEFAULT_HEARTBEAT_INTERVAL      (20 * 1000)

#define MAX_HEARBEAT_RESPONCE_SIZE      8192

extern char *the_client_id;

extern char *tmp_client_id;

extern unsigned int the_hb_interval;

extern jvalue *jconfig;

extern jvalue *jsettings;

extern jvalue *jmetadata;

extern ccl_platform_interface *pinterface;

extern jvalue *platform_metadata(const ccl_metadata *platform);

extern jvalue *settings_metadata(const ccl_metadata *settings);

extern int load_persistent_data(void);

extern int save_persistent_data(void);

#endif
