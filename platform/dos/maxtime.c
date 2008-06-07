#include "../maxtime.h"
#include "../../boolean.h"
#include "../../pymsg.h"
#include <limits.h>

void initMaxtime(void)
{
  /* no initialization necessary */
}

void setMaxtime(unsigned int *seconds)
{
  if (*seconds<UINT_MAX)
  {
    VerifieMsg(NoMaxTime);
    maxtime_status = MAXTIME_TIMEOUT;
  }
  else
    maxtime_status = MAXTIME_IDLE;
}
