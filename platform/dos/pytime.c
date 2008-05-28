#include "../pytime.h"

/* Various implementations for DOS compilers */

#	if defined(__TURBOC__)
/* We do not use this code since it requires floating point
** arithmetic. This would be the only place we need it.
** Under DOS we can also use the time routine since we have
** no multiprocessing under DOS, :-(
**
** #	   include <time.h>
** #	   include <math.h>
**
** static  clock_t ThisTime;
** void StartTimer(void)
** {
**	ThisTime= clock();
** }
**
** long StopTimer(void)
** {
**	return (clock() - ThisTime)/CLK_TCK;
** }
*/

#	include <time.h>

static time_t ThisTime;

void StartTimer(void) {
    ThisTime= time((time_t *)0);
}

long StopTimer(void) {
    return time((time_t *)0) - ThisTime;
}
#	else	/* not __TURBOC__ */

#	if defined(_MSC_VER)
/* we are using MicroSoft C Compiler */

#include <time.h>

static clock_t ThisTime;

void StartTimer(void) {
    ThisTime= clock();
}

long StopTimer(void) {
    return (clock() - ThisTime)/CLK_TCK;
}

#	else /* not _MSC_VER */
#	if defined(__GNUC__)
	/* we are using GCC */

#	include <time.h>

static time_t ThisTime;

void StartTimer(void) {
    ThisTime= time((time_t *)0);
}

long StopTimer(void) {
    return time((time_t *)0) - ThisTime;
}
#	else	/* not __GNUC__ */

/* we are using any other C Compiler */

#include <time.h>

static clock_t ThisTime;

void StartTimer(void) {
    ThisTime= clock();
}

long StopTimer(void) {
    return (clock() - ThisTime)/CLK_TCK;
}

#endif /* __GNUC__ */
#endif /* _MSC_VER */
#endif /* __TURBOC__ */
