#if !defined(MAXTIME_H)
#define MAXTIME_H

#include "boolean.h"

#include <signal.h>

/* number of periods passed since timer started */
sig_atomic_t volatile periods_counter;

/* number of periods after which solving is aborted */
sig_atomic_t volatile nr_periods;


typedef unsigned int maxtime_type;

/* Maximum number of seconds of maxtime supported by the platform.
 * Guaranteed to be initialized after initMaxTime() has returned.
 */
extern maxtime_type maxtime_maximum_seconds;

enum
{
  /* singular value for indicating that a maxtime value is not set */
  no_time_set = 0
};


/* Platform-dependant initialization of the maxtime machinery. To be
 * called once at program start.
 */
void initMaxtime(void);

/* Inform the maxtime module about the value of the -maxtime command
 * line parameter
 * @param commandlineValue value of the -maxtime command line parameter
 */
void setCommandlineMaxtime(maxtime_type commandlineValue);

/* Reset the value of the maxtime option.
 * To be called whenever the value set with option maxtime becomes obsolete.
 */
void resetOptionMaxtime(void);

/* Store the value of the option maxtime.
 * @param optionValue value of the option maxtime
 */
void setOptionMaxtime(maxtime_type optionValue);

/* Platform-dependant function for setting the maximal solving time.
 * Don't call directly; this function is called from dealWithMaxtime().
 *
 * If seconds==no_time_set, sets periods_counter<nr_periods; otherwise
 * periods_counter will be assigned a value >=nr_periods after seconds
 * seconds unless setMaxtime() is called again in the meantime.
 *
 * @param seconds number of seconds until periods_counter is to be set
 *                to a value >= nr_periods
 */
void setMaxtime(maxtime_type seconds);

/* Set the appropriate maximal solving time based on the command line
 * paramter and option maxtime value.
 * @return true iff a maximum solving time has been set
 */
boolean dealWithMaxtime(void);

#endif
