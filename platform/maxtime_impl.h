#if !defined(MAXTIME_IMPL_H)
#define MAXTIME_IMPL_H

#include "platform/maxtime.h"
#include <signal.h>

/* number of periods passed since timer started */
sig_atomic_t volatile periods_counter;

/* number of periods after which solving is aborted */
sig_atomic_t volatile nr_periods;


/* Maximum number of seconds of maxtime supported by the platform.
 * Guaranteed to be initialized after initMaxTime() has returned.
 */
extern maxtime_type maxtime_maximum_seconds;

enum
{
  /* singular value for indicating that a maxtime value is not set */
  no_time_set = 0
};


#endif
