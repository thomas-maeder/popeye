#include "../maxtime.h"
#include "../../boolean.h"
#include <limits.h>

void initMaxtime(void)
{
  /* no initialization necessary */
}

#if defined(_MT)

#include <windows.h>
#include <process.h>

#include <stdio.h>

sig_atomic_t volatile currentProblem = 0;

static void solvingTimeMeasureThread(void *v)
{
  unsigned int *seconds = (unsigned int *)v;
  
  /*
   * This function is used by a WIN32-thread to wake up
   * the thread after WaitTime seconds.
   */
  sig_atomic_t const myProblem = currentProblem;

  /* sleep under WIN32 seems to use milliseconds ... */
  _sleep(*seconds*1000);

  if (myProblem==currentProblem)
    maxtime_status = MAXTIME_TIMEOUT;

  _endthread();
}

void setMaxtime(unsigned int *seconds)
{
  if (*seconds==UINT_MAX)
    maxtime_status = MAXTIME_IDLE;
  else
  {
    maxtime_status = MAXTIME_TIMING;

    /* To avoid that a not "used" thread stops Popeye when it times out,
     * currentProblem is increased every time a new problem is to
     * be solved.
     * TODO: kill thread when problem is fully solved before timeout?
     */
    ++currentProblem;

    /* this is the reason why we impose a restriction on *seconds in
     * maxtime.h: *seconds must still have its value when it is read
     * from within solvingTimeMeasureThread(). */
    _beginthread(&solvingTimeMeasureThread,0,seconds);
  }
}

#else

#include "../../pymsg.h"

void setMaxtime(unsigned int *seconds)
{
  maxtime_status = MAXTIME_IDLE;

  if (*seconds<UINT_MAX)
    VerifieMsg(NoMaxTime);
}

#endif
