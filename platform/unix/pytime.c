#include "platform/timer.h"
#include <time.h>

/* depending on the platform, clock() may wrap very frequently
 * (e.g. with a period only slightly above 1h), so let's use a combo
 * of clock() and time().
 * If we are extremely unlucky, we may be off by 1s.
 */

static clock_t startClock;
static time_t startTime;

void StartTimer(void)
{
  startClock = clock();
  startTime = time(0);
}

void StopTimer(unsigned long *seconds, unsigned long *msec)
{
  clock_t stopClock = clock();
  time_t stopTime = time(0);

  /* there are implementations with CLOCKS_PER_SEC of type double! */
  clock_t ticks_per_sec = (clock_t)CLOCKS_PER_SEC;
  clock_t ticks = stopClock-startClock;
  *msec = (unsigned long)((ticks%ticks_per_sec) * 1000 / ticks_per_sec);

  *seconds = (unsigned long)(stopTime-startTime);
}
