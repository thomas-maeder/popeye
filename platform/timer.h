#if !defined(PLATFORM_TIMER_H)
#define PLATFORM_TIMER_H

#include "stipulation/stipulation.h"

#include <limits.h>

enum
{
  /* Ideally, this would be ULONG_MAX, but ISO C restricts enumerator
   * values to range of int. In practice, anything >=1000 works. */
  MSEC_NOT_SUPPORTED = INT_MAX
};


/* Start the timer for measuring the solving time for one problem.
 */
void StartTimer(void);


/* Stop the timer for measuring the solving time for one problem.
 * @param seconds address of object that is assigned the number of
 *                seconds since the last call of StartTimer()
 * @param msec address of object that is assigned the number of
 *             milliseconds (0-999, to be added to *seconds) since
 *             the last call of StartTimer()
 * @note *msec is assigned MSEC_NOT_SUPPORTED if the platform doesn't
 *       support timing more accurately than in seconds
 */
void StopTimer(unsigned long *seconds, unsigned long *msec);

void timer_starter_solve(slice_index si);

#endif /* PYTIME_H */
