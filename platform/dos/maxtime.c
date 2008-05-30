#include "../maxtime.h"
#include "../../boolean.h"
#include "../../pymsg.h"

void initMaxtime(void)
{
  /* no initialization necessary */
}

void setMaxtime(unsigned int *seconds)
{
  VerifieMsg(NoMaxTime);
  FlagTimeOut = true;
}
