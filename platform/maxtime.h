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


/* Initialize maxtime machinery. To be called once at program start.
 */
void initMaxtime(void);

/* Set the maximal solving time for the problem or twin to be solved
 * next.
 * If (*seconds==UINT_MAX), sets maxtime_status to MAXTIME_IDLE;
 * otherwise sets maxtime_status to MAXTIME_TIMING, and to
 * MAXTIME_TIMEOUT after the requested number of seconds unless
 * setMaxtime() has been called again in the meantime.
 * @param seconds address of variable holding number of seconds until
 *                maxtime_status is to be assigned MAXTIME_TIMEOUT; the
 *                variable must live (and keep its value) until the
 *                problem is solved or solving is aborted because
 *                *seconds seconds have elapsed (whichever is shorter).
 *                If (*seconds==UINT_MAX), timing is deactivated for the
 *                problem/twin.
 */
void setMaxtime(unsigned int *seconds);

#endif
