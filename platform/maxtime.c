#include "maxtime.h"
#include "../pydata.h"

static maxtime_type maxTimeCommandLine = no_time_set;

static maxtime_type maxTimeOption = no_time_set;

sig_atomic_t volatile maxtime_status = MAXTIME_IDLE;

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
 */
void dealWithMaxtime(void)
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
}
