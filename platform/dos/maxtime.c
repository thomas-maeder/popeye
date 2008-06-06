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
  maxtime_status = MAXTIME_IDLE;

  if (*seconds<UINT_MAX)
    VerifieMsg(NoMaxTime);
}
