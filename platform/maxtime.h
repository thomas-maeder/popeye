#if !defined(MAXTIME_H)
#define MAXTIME_H

#include <signal.h>

void initMaxtime(void);

/* The semantics of the following two flags isn't entirely clear, but
 * from what I can tell, it is a very good idea to make them "as
 * atomic as possible". (TM 20080529)
 */
extern sig_atomic_t volatile FlagTimeOut;
extern sig_atomic_t volatile FlagTimerInUse;

void setMaxtime(unsigned int seconds);

#endif
