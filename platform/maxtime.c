#if !defined(MAX_NR_PERIODS)
#  if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) && \
      !(defined(__cplusplus) && (__cplusplus >= 201103L))
#    if !defined(__STDC_LIMIT_MACROS)
#      define __STDC_LIMIT_MACROS
#    endif
#  endif
#endif

#include "maxtime_impl.h"
#include "debugging/trace.h"

#if !defined(MAX_NR_PERIODS)
#  if !defined(SIG_ATOMIC_MAX)
#    if defined(__cplusplus) && (__cplusplus >= 201103L)
#      include <cstdint>
#    else
#      if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#        include <stdint.h>
#      endif
#    endif
#  endif
#  if defined(SIG_ATOMIC_MAX)
#    define MAX_NR_PERIODS SIG_ATOMIC_MAX
#  else
#    if defined(__cplusplus)
#      include <climits>
#    else
#      include <limits.h>
#    endif
#    if defined(LLONG_MAX)
#      define MAX_NR_PERIODS ((((sig_atomic_t)-1)>0) ? \
                              ((sig_atomic_t)-1) : \
                              ((sizeof(sig_atomic_t)>=sizeof(long long)) ? \
                               ((sig_atomic_t)LLONG_MAX) : \
                               ((sizeof(sig_atomic_t)>=sizeof(long)) ? \
                                ((sig_atomic_t)LONG_MAX) : \
                                ((sizeof(sig_atomic_t)>=sizeof(int)) ? \
                                 ((sig_atomic_t)INT_MAX) : \
                                 ((sizeof(sig_atomic_t)>=sizeof(short)) ? \
                                  ((sig_atomic_t)SHRT_MAX) : \
                                  ((sig_atomic_t)SCHAR_MAX))))))
#    else
#      define MAX_NR_PERIODS ((((sig_atomic_t)-1)>0) ? \
                              ((sig_atomic_t)-1) : \
                              ((sizeof(sig_atomic_t)>=sizeof(long)) ? \
                               ((sig_atomic_t)LONG_MAX) : \
                               ((sizeof(sig_atomic_t)>=sizeof(int)) ? \
                                ((sig_atomic_t)INT_MAX) : \
                                ((sizeof(sig_atomic_t)>=sizeof(short)) ? \
                                 ((sig_atomic_t)SHRT_MAX) : \
                                 ((sig_atomic_t)SCHAR_MAX)))))
#    endif
#  endif
#endif

static maxtime_type maxTimeCommandLine = no_time_set;

static maxtime_type maxTimeOption = no_time_set;

/* number of seconds passed since timer started */
sig_atomic_t volatile periods_counter = 0;

/* maximum allowed value of nr_periods */
sig_atomic_t const max_nr_periods = MAX_NR_PERIODS;

/* number of seconds after which solving is aborted */
sig_atomic_t volatile nr_periods = MAX_NR_PERIODS;

/* Inform the maxtime module about the value of the -maxtime command
 * line parameter
 * @param commandlineValue value of the -maxtime command line parameter
 */
void platform_set_commandline_maxtime(maxtime_type commandlineValue)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",commandlineValue);
  TraceFunctionParamListEnd();

  maxTimeCommandLine = commandlineValue;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Reset the value of the maxtime option.
 * To be called whenever the value set with option maxtime becomes obsolete.
 */
void platform_reset_option_maxtime(void)
{
  maxTimeOption = no_time_set;
}

/* Set the value of the maxtime option.
 * To be called whenever the value set with option maxtime becomes obsolete.
 * @param m value to be set
 */
void platform_set_option_maxtime(maxtime_type m)
{
  maxTimeOption = m;
}

boolean platform_is_maxtime_set(void)
{
  return maxTimeOption!=no_time_set || maxTimeCommandLine!=no_time_set;
}

/* Set the appropriate maximal solving time based on the command line
 * paramter and option maxtime value.
 * @return true iff a maximum solving time has been set
 */
boolean platform_deal_with_maxtime(void)
{
  boolean result;

  /* If a maximal time is indicated both on the command line and as
   * an option, use the smaller value.
   */
  if (maxTimeCommandLine==no_time_set)
    result = platform_set_maxtime_timer(maxTimeOption);
  else if (maxTimeOption==no_time_set)
    result = platform_set_maxtime_timer(maxTimeCommandLine);
  else if (maxTimeCommandLine<maxTimeOption)
    result = platform_set_maxtime_timer(maxTimeCommandLine);
  else
    result = platform_set_maxtime_timer(maxTimeOption);

  return result;
}

/* Has the set maximum time elapsed
 * @return truee iff the set maximum time has elapsed
 */
boolean platform_has_maxtime_elapsed(void)
{
  return periods_counter>=nr_periods;
}
