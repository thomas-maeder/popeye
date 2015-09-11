#include "platform/timer.h"

/* Various implementations for DOS compilers */

#if defined(__TURBOC__)
/* We do not use this code since it requires floating point
** arithmetic. This would be the only place we need it.
** Under DOS we can also use the time routine since we have
** no multiprocessing under DOS, :-(
**
** #       include <time.h>
** #       include <math.h>
**
** static  clock_t ThisTime;
** void StartTimer(void)
** {
**  ThisTime= clock();
** }
**
** long StopTimer(void)
** {
**  return (clock() - ThisTime)/CLK_TCK;
** }
*/

#include <time.h>

static time_t StartTime;

void StartTimer(void)
{
  StartTime= time((time_t *)0);
}

void StopTimer(unsigned long *seconds, unsigned long *msec)
{
  *seconds = time((time_t *)0) - StartTime;
  *msec = MSEC_NOT_SUPPORTED;
}

#elif defined(_MSC_VER)
/* we are using MicroSoft C Compiler */

#include <time.h>

static clock_t StartTime;

void StartTimer(void)
{
  StartTime = clock();
}

void StopTimer(unsigned long *seconds, unsigned long *msec)
{
  *seconds = (clock() - StartTime)/CLK_TCK;
  *msec = MSEC_NOT_SUPPORTED;
}

#elif defined(__GNUC__)
/* we are using GCC */

#include <time.h>

static time_t StartTime;

void StartTimer(void)
{
  StartTime= time((time_t *)0);
}

void StopTimer(unsigned long *seconds, unsigned long *msec)
{
  *seconds = time((time_t *)0) - StartTime;
  *msec = MSEC_NOT_SUPPORTED;
}

#else   /* !__GNUC__ */

/* we are using any other C Compiler */

#include <time.h>

static clock_t StartTime;

void StartTimer(void)
{
  StartTime= clock();
}

void StopTimer(unsigned long *seconds, unsigned long *msec)
{
  *seconds = (clock() - StartTime)/CLK_TCK;
  *msec = MSEC_NOT_SUPPORTED;
}

#endif /* !__GNUC__ */
