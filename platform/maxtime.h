#if !defined(MAXTIME_H)
#define MAXTIME_H

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

typedef unsigned int maxtime_type;

/* Inform the maxtime module about the value of the -maxtime command
 * line parameter
 * @param commandlineValue value of the -maxtime command line parameter
 */
void setCommandlineMaxtime(maxtime_type commandlineValue);

/* Reset the value of the maxtime option.
 * To be called whenever the value set with option maxtime becomes obsolete.
 */
void resetOptionMaxtime(void);

/* Propagate our findings to STOptionInterruption
 * @param si identifies the slice where to start instrumenting
 */
void maxtime_propagator_solve(slice_index si);

/* Instrument the solving machinery with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 * @param maxtime
 */
void maxtime_instrument_solving(slice_index si, maxtime_type maxtime);

boolean isMaxtimeSet(void);

void resetMaxtimeTimer(void);

/* Platform-dependant function for setting the maximal solving time.
 * Don't call directly; this function is called from dealWithMaxtime().
 *
 * If seconds==no_time_set, sets periods_counter<nr_periods; otherwise
 * periods_counter will be assigned a value >=nr_periods after seconds
 * seconds unless setMaxtimeTimer() is called again in the meantime.
 *
 * @param seconds number of seconds until periods_counter is to be set
 *                to a value >= nr_periods
 */
boolean setMaxtimeTimer(maxtime_type seconds);

/* Set the appropriate maximal solving time based on the command line
 * paramter and option maxtime value.
 * @return true iff a maximum solving time has been set
 */
boolean dealWithMaxtime(void);

/* Has the set maximum time elapsed
 * @return truee iff the set maximum time has elapsed
 */
boolean hasMaxtimeElapsed(void);

#endif
