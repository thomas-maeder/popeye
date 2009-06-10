#include "../maxtime.h"
#include "../../boolean.h"
#include "../../pymsg.h"

#include <assert.h>
#include <limits.h>

/* Implementation of option maxtime using Windows Multimedia timers
 */

/* number of seconds passed since timer started */
static sig_atomic_t volatile seconds_counter = 0;

/* number of seconds after which solving is aborted */
static sig_atomic_t volatile seconds_maxtime = 0;

/* Singular value indicating that an MMRESULT object does not hold an
 * actual timer resource.
 * The docs
 * (http://msdn.microsoft.com/en-us/library/ms713423(VS.85).aspx) say
 * that timeSetEvent() returns NULL if it fails; but MMRESULT is not a
 * pointer type ...
 */
static MMRESULT const no_timer = 0;

/* id of current Multimedia Timer
 */
static MMRESULT current_timer;

/* Actual timer resolution; initialized with intended resolution
 */
static unsigned int actual_timer_resolutionMS = 100;

/* Singular value that we use to indicate that the Multimedia Timers
 * machinery is not available.
 */
static unsigned int const MM_timers_not_available = 0;

/* Calibrate timer resolution based on intended resolution and
 * capabilities indicated by Windows.
 */
static void calibrate_timer_resolution(void)
{
  TIMECAPS tc;
  if (timeGetDevCaps(&tc, sizeof tc) == TIMERR_NOERROR) 
  {
    if (actual_timer_resolutionMS<tc.wPeriodMin)
      actual_timer_resolutionMS = tc.wPeriodMin;
    else if (actual_timer_resolutionMS>tc.wPeriodMax)
      actual_timer_resolutionMS = tc.wPeriodMax;
  }
  else
    actual_timer_resolutionMS = MM_timers_not_available;
}

/* Callback function for Multimedia Timer.
 * Called back by Windows every actual_timer_delayMS milliseconds
 */
static void CALLBACK tick(UINT timer_id,      
                          UINT reserved1,     
                          DWORD seconds_elapsed,  
                          DWORD reserved2,     
                          DWORD reserved3)
{
  /* assert(timer_id==current_timer);
   * is probably not a good idea in (something similar to) a signal
   * handler ...
   */

  seconds_counter += seconds_elapsed;
  if (seconds_counter>=seconds_maxtime)
    maxtime_status = MAXTIME_TIMEOUT;
}

/* Attempt to set up a new timer for timing out solving after a number
 * of seconds. 
 * @param seconds number of seconds after which solving is aborted
 */
static void setupNewTimer(unsigned int seconds)
{
  unsigned int const min_seconds_elapsed = actual_timer_resolutionMS*10/1000;
  DWORD const seconds_elapsed = min_seconds_elapsed==0 ? 1 : min_seconds_elapsed;
  unsigned int const delayMS = seconds_elapsed*1000;

  /* according to the docs
   * (http://msdn.microsoft.com/en-us/library/ms713423(VS.85).aspx),
   * it would be a good idea to also specify TIME_KILL_SYNCHRONOUS,
   * but the MingW based cross compiler doesn't 'know' that symbol.
   */
  current_timer = timeSetEvent(delayMS,actual_timer_resolutionMS,
                               &tick,seconds_elapsed,
                               TIME_PERIODIC);
  if (current_timer==no_timer)
  {
    VerifieMsg(NoMaxTime);
    maxtime_status = MAXTIME_TIMEOUT;
  }
  else
  {
    maxtime_status = MAXTIME_IDLE;
    seconds_counter = 0;
    seconds_maxtime = seconds;
  }
}

/* Kill previously used timer resource (if any)
 */
static void killPreviousTimer(void)
{
  if (current_timer!=no_timer)
  {
    MMRESULT const kill_result = timeKillEvent(current_timer);
    assert(kill_result==TIMERR_NOERROR);
    current_timer = no_timer;
  }
}

void initMaxtime(void)
{
  calibrate_timer_resolution();
  current_timer = no_timer;
}

void setMaxtime(unsigned int *seconds)
{
    killPreviousTimer();

    if (*seconds==UINT_MAX)
      maxtime_status = MAXTIME_IDLE;
    else if (actual_timer_resolutionMS==MM_timers_not_available)
    {
      VerifieMsg(NoMaxTime);
      maxtime_status = MAXTIME_TIMEOUT;
    }
    else
      setupNewTimer(*seconds);
}
