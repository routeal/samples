#include "ccl.h"

/**
 * Creates a platform interface.
 */
extern const ccl_platform_t *platform_create(unsigned int session_size);

/**
 * Destroys a platform interface.
 */
extern void platform_destroy(void);
