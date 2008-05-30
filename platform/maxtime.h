#if !defined(MAXTIME_H)
#define MAXTIME_H

#include <signal.h>

/* The semantics of the following two flags isn't entirely clear, but
 * from what I can tell, it is a very good idea to make them "as
 * atomic as possible" to allow them to be accessed from different
 * threads or a signal handler. (TM 20080529)
 */
extern sig_atomic_t volatile FlagTimeOut;
extern sig_atomic_t volatile FlagTimerInUse;

/* Initialize maxtime machinery. To be called once at program start.
 */
void initMaxtime(void);

/* Set the maximal solving time for the problem to be solved next.
 * Sets FlagTimeOut after the requested number of seconds unless
 * setMaxtime() has been called again in the meantime.
 * @param seconds address of variable holding number of seconds until
 *                FlagTimeOut is to be set; the variable must live
 *                (and keep its value) until the problem is solved or
 *                solving is aborted because *seconds seconds have
 *                elapsed (whichever is shorter).
 */
void setMaxtime(unsigned int *seconds);

#endif
