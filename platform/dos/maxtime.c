#include "platform/maxtime_impl.h"
#include "utilities/boolean.h"
#include "output/plaintext/message.h"
#include <limits.h>

void initMaxtime(void)
{
  maxtime_maximum_seconds = UINT_MAX;
}

void setMaxtime(maxtime_type seconds)
{
  if (seconds==no_time_set)
  {
    periods_counter = 0;
    nr_periods = 1;
  }
  else
  {
    VerifieMsg(NoMaxTime);
    periods_counter = 1;
    nr_periods = 0;
  }
}
