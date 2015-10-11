#if !defined(MAXTIME_H)
#define MAXTIME_H

#include "utilities/boolean.h"

typedef unsigned int maxtime_type;

/* Inform the maxtime module about the value of the -maxtime command
 * line parameter
 * @param commandlineValue value of the -maxtime command line parameter
 */
void platform_set_commandline_maxtime(maxtime_type commandlineValue);

/* Reset the value of the maxtime option.
 * To be called whenever the value set with option maxtime becomes obsolete.
 */
void platform_reset_option_maxtime(void);

/* Set the value of the maxtime option.
 * To be called whenever the value set with option maxtime becomes obsolete.
 * @param m value to be set
 */
void platform_set_option_maxtime(maxtime_type m);

boolean platform_is_maxtime_set(void);

void platform_reset_maxtime_timer(void);

/* Platform-dependant function for setting the maximal solving time.
 * Don't call directly; this function is called from platform_deal_with_maxtime().
 *
 * If seconds==no_time_set, sets periods_counter<nr_periods; otherwise
 * periods_counter will be assigned a value >=nr_periods after seconds
 * seconds unless platform_set_maxtime_timer() is called again in the meantime.
 *
 * @param seconds number of seconds until periods_counter is to be set
 *                to a value >= nr_periods
 */
boolean platform_set_maxtime_timer(maxtime_type seconds);

/* Set the appropriate maximal solving time based on the command line
 * paramter and option maxtime value.
 * @return true iff a maximum solving time has been set
 */
boolean platform_deal_with_maxtime(void);

/* Has the set maximum time elapsed
 * @return truee iff the set maximum time has elapsed
 */
boolean platform_has_maxtime_elapsed(void);

#endif
