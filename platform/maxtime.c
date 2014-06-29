#include "maxtime_impl.h"

#include <limits.h>

static maxtime_type maxTimeCommandLine = no_time_set;

static maxtime_type maxTimeOption = no_time_set;

/* number of seconds passed since timer started */
sig_atomic_t volatile periods_counter = 0;

/* number of seconds after which solving is aborted */
sig_atomic_t volatile nr_periods = UINT_MAX;

/* Inform the maxtime module about the value of the -maxtime command
 * line parameter
 * @param commandlineValue value of the -maxtime command line parameter
 */
void setCommandlineMaxtime(maxtime_type commandlineValue)
{
  maxTimeCommandLine = commandlineValue;
}

/* Reset the value of the maxtime option.
 * To be called whenever the value set with option maxtime becomes obsolete.
 */
void resetOptionMaxtime(void)
{
  maxTimeOption = no_time_set;
}

boolean isMaxtimeSet(void)
{
  return maxTimeOption!=no_time_set || maxTimeCommandLine!=no_time_set;
}

/* Store the value of the option maxtime.
 * @param optionValue value of the option maxtime
 * @return true iff the value is valid and could be set
 */
void setOptionMaxtime(maxtime_type optionValue)
{
  maxTimeOption = optionValue;
}

/* Set the appropriate maximal solving time based on the command line
 * paramter and option maxtime value.
 * @return true iff a maximum solving time has been set
 */
boolean dealWithMaxtime(void)
{
  boolean result;

  /* If a maximal time is indicated both on the command line and as
   * an option, use the smaller value.
   */
  if (maxTimeCommandLine==no_time_set)
    result = setMaxtime(maxTimeOption);
  else if (maxTimeOption==no_time_set)
    result = setMaxtime(maxTimeCommandLine);
  else if (maxTimeCommandLine<maxTimeOption)
    result = setMaxtime(maxTimeCommandLine);
  else
    result = setMaxtime(maxTimeOption);

  return result;
}

/* Has the set maximum time elapsed
 * @return truee iff the set maximum time has elapsed
 */
boolean hasMaxtimeElapsed(void)
{
  return periods_counter>=nr_periods;
}
