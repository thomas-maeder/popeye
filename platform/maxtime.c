#include "maxtime_impl.h"
#include "pydata.h"

static maxtime_type maxTimeCommandLine = no_time_set;

static maxtime_type maxTimeOption = no_time_set;

/* number of seconds passed since timer started */
sig_atomic_t volatile periods_counter = 0;

/* number of seconds after which solving is aborted */
sig_atomic_t volatile nr_periods = UINT_MAX;

/* Maximum number of seconds of maxtime supported by the platform.
 * Guaranteed to be initialized after initMaxTime() has returned.
 */
maxtime_type maxtime_maximum_seconds;

/* Retrieve the maximum number of seconds supported by the platform.
 * Guaranteed to be initialized only after initMaxTime() has returned.
 * @return maximum number of seconds supported by the platform
 */
maxtime_type maxtimeMaximumSeconds(void)
{
  return maxtime_maximum_seconds;
}

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

/* Store the value of the option maxtime.
 * @param optionValue value of the option maxtime
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
  /* If a maximal time is indicated both on the command line and as
   * an option, use the smaller value.
   */
  if (maxTimeCommandLine==no_time_set)
    setMaxtime(maxTimeOption);
  else if (maxTimeOption==no_time_set)
    setMaxtime(maxTimeCommandLine);
  else if (maxTimeCommandLine<maxTimeOption)
    setMaxtime(maxTimeCommandLine);
  else
    setMaxtime(maxTimeOption);

  return maxTimeCommandLine!=no_time_set || maxTimeOption!=no_time_set;
}

/* Has the set maximum time elapsed
 * @return truee iff the set maximum time has elapsed
 */
boolean hasMaxtimeElapsed(void)
{
  return periods_counter>=nr_periods;
}
