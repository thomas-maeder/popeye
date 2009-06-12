#if !defined(MAXTIME_H)
#define MAXTIME_H

#include <signal.h>

/* Possible states of the maxtime machinery
 * The order of the enumerators was selected because
 * maxtime_status==MAXTIME_TIMEOUT is expected to be a frequent
 * operation.
 */
enum
{
  MAXTIME_TIMEOUT,
  MAXTIME_TIMING,
  MAXTIME_IDLE
};

/* Current state of the maxtime machinery
 * @note: read-only from outside the maxtime implementation
 */
extern sig_atomic_t volatile maxtime_status;


typedef unsigned long maxtime_type;

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
 * If seconds==UINT_MAX, sets maxtime_status to MAXTIME_IDLE;
 * otherwise sets maxtime_status to MAXTIME_TIMING, and to
 * MAXTIME_TIMEOUT after the requested number of seconds unless
 * setMaxtime() has been called again in the meantime.
 * @param seconds number of seconds until maxtime_status is to be
 *                assigned MAXTIME_TIMEOUT
 */
void setMaxtime(maxtime_type seconds);

/* Set the appropriate maximal solving time based on the command line
 * paramter and option maxtime value.
 */
void dealWithMaxtime(void);

#endif
