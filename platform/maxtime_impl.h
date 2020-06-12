#if !defined(MAXTIME_IMPL_H)
#define MAXTIME_IMPL_H

#include "platform/maxtime.h"
#include <signal.h>

/* Option maxtime: interface for (platform-dependant) implementations
 */

/* number of periods passed since timer started */
extern sig_atomic_t volatile periods_counter;

/* number of periods after which solving is aborted */
extern sig_atomic_t volatile nr_periods;

/* maximum allowed value of nr_periods */
extern sig_atomic_t const max_nr_periods;

enum
{
  /* singular value for indicating that a maxtime value is not set */
  no_time_set = 0
};


#endif
