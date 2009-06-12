#include "../maxtime.h"
#include "../../boolean.h"
#include "../../pymsg.h"
#include <limits.h>

void initMaxtime(void)
{
  /* no initialization necessary */
}

void setMaxtime(maxtime_type seconds)
{
  if (seconds==no_time_set)
    maxtime_status = MAXTIME_IDLE;
  else
  {
    VerifieMsg(NoMaxTime);
    maxtime_status = MAXTIME_TIMEOUT;
  }
}
