#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) && \
    !(defined(__cplusplus) && (__cplusplus >= 201103L))
#  if !defined(__STDC_LIMIT_MACROS)
#    define __STDC_LIMIT_MACROS
#  endif
#endif

#include "platform/maxtime_impl.h"
#include <windows.h>
#include "utilities/boolean.h"

#include "debugging/assert.h"

#if !defined(SIG_ATOMIC_MAX)
#  if defined(__cplusplus) && (__cplusplus >= 201103L)
#    include <cstdint>
#  else
#    if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#      include <stdint.h>
#    endif
#    if !defined(SIG_ATOMIC_MAX)
#      if defined(__cplusplus)
#        include <climits>
#      else
#        include <limits.h>
#      endif
#      if defined(LLONG_MAX)
#        define SIG_ATOMIC_MAX ((((sig_atomic_t) -1) > 0) ? \
                                ((sig_atomic_t) -1) : \
                                ((sizeof(sig_atomic_t)==sizeof(int)) ? \
                                 INT_MAX : \
                                 ((sizeof(sig_atomic_t)==sizeof(short)) ? \
                                  SHRT_MAX : \
                                  ((sizeof(sig_atomic_t)==sizeof(long)) ? \
                                   LONG_MAX : \
                                   ((sizeof(sig_atomic_t)==sizeof(long long)) ? \
                                    LLONG_MAX : \
                                    SCHAR_MAX)))))
#      else
#        define SIG_ATOMIC_MAX ((((sig_atomic_t) -1) > 0) ? \
                                ((sig_atomic_t) -1) : \
                                ((sizeof(sig_atomic_t)==sizeof(int)) ? \
                                 INT_MAX : \
                                 ((sizeof(sig_atomic_t)==sizeof(short)) ? \
                                  SHRT_MAX : \
                                  ((sizeof(sig_atomic_t)==sizeof(long)) ? \
                                   LONG_MAX : \
                                   SCHAR_MAX))))
#      endif
#    endif
#  endif
#endif

/* Implementation of option maxtime using Windows Multimedia timers
 */

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

/* The Windows APIs use all kinds of types with strange Hungarian
 * names, but it all boils down to:
 */
typedef unsigned long timer_period_type;

/* Actual timer resolution; initialized with intended resolution
 */
static timer_period_type timer_resolutionMS = 1000;

/* Singular value that we use to indicate that the Multimedia Timers
 * machinery is not available.
 */
static timer_period_type const MM_timers_not_available = 0;

/* we support up to UINT_MAX milliseconds (i.e. ~1 month)
 */
static maxtime_type maxtime_maximum_seconds = UINT_MAX/1000;

/* Calibrate timer resolution based on intended resolution and
 * capabilities indicated by Windows.
 */
static void calibrate_timer_resolution(void)
{
  TIMECAPS tc;
  if (timeGetDevCaps(&tc, sizeof tc) == TIMERR_NOERROR)
  {
    if (timer_resolutionMS<tc.wPeriodMin)
      timer_resolutionMS = tc.wPeriodMin;
    else if (timer_resolutionMS>tc.wPeriodMax)
      timer_resolutionMS = tc.wPeriodMax;
  }
  else
    timer_resolutionMS = MM_timers_not_available;
}

/* Callback function for Multimedia Timer.
 * Called back by Windows every timer_delayMS milliseconds
 */
static void CALLBACK tick(unsigned int timer_id,
                          unsigned int reserved1,
                          unsigned long long unused_user_data,
                          unsigned long long reserved2,
                          unsigned long long reserved3)
{
  /* assert(timer_id==current_timer);
   * is probably not a good idea in (something similar to) a signal
   * handler ...
   */

  if (periods_counter < SIG_ATOMIC_MAX)
    ++periods_counter;
}

/* Attempt to set up a new timer for timing out solving after a number
 * of seconds.
 * @param seconds number of seconds after which solving is aborted
 */
static boolean setupNewTimer(maxtime_type seconds)
{
  unsigned int const intended_period_lengthMS = 1000;
  unsigned int const period_lengthMS = (intended_period_lengthMS
                                        <timer_resolutionMS
                                        ? timer_resolutionMS
                                        : intended_period_lengthMS);

  unsigned int const milliseconds = 1000*seconds;
  nr_periods = milliseconds/period_lengthMS;

  current_timer = timeSetEvent(period_lengthMS,timer_resolutionMS,
                               &tick,0,
                               TIME_PERIODIC
                               |TIME_KILL_SYNCHRONOUS
                               |TIME_CALLBACK_FUNCTION);
  if (current_timer==no_timer)
  {
    nr_periods = 0;
    periods_counter = 1;
    return false;
  }
  else
  {
    periods_counter = 0;
    return true;
  }
}

void platform_reset_maxtime_timer(void)
{
  if (current_timer!=no_timer)
  {
#if !defined(NDEBUG)
    MMRESULT const kill_result =
#endif
    timeKillEvent(current_timer);
    assert(kill_result==TIMERR_NOERROR);
    current_timer = no_timer;
  }
}

boolean platform_set_maxtime_timer(maxtime_type seconds)
{
  assert(current_timer==no_timer);

  if (seconds==no_time_set)
  {
    nr_periods = 1;
    periods_counter = 0;
    return true;
  }
  else if (seconds>maxtime_maximum_seconds)
  {
    nr_periods = 0;
    periods_counter = 1;
    return true;
  }
  else
  {
    calibrate_timer_resolution();
    if (timer_resolutionMS==MM_timers_not_available)
    {
      nr_periods = 0;
      periods_counter = 1;
      return true;
    }
    else
      return setupNewTimer(seconds);
  }
}

void platform_init(void)
{
}
