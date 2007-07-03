/******************** MODIFICATIONS to pytime.c ************************
**
** Date        Who  What
**
** 2001/11/10 NG   Moved all remarks on modifications of pytime.c to
**                 pytime-c.mod. Look there for remarks before 2001/01/01.
**
**************************** End of List ******************************/

/* This is pytime.c
** It is a new module that comes with py2.0c and is highly
** OperatingSystem dependent. Two procedures are defined here:
** StartTime and StopTime.
** These procedures serve the purpose to keep the rest of
** popeye clean from OS dependencies.
** Description of the functions:
**      void StartTime():
**        Starts the timer. If compiled for unix this is
**        done via the times(3C)-call. It records the amount
**        of time popeye has consumed up to the call of this
**        function.
**      long Stoptime():
**        This function delivers the time passed since the last
**        call to StopTime. The unit of measure are seconds.
**        If there was no previous call to StartTimer the result
**        is undefined (in popeye this never happens).
** If there is need to reimplement these two functions, then
** doit and add a #ifdef which can be selected in the Makefile.
**
*/

#ifdef NOVOID
#       define  void    int
#endif

#ifdef UNIX
#       include <sys/types.h>
#       include <sys/param.h>
#       include <sys/times.h>
#	include	<unistd.h>

/*
 * There may be systems, where HZ is not defined in param.h.
 * Especially BSD-System lack this constant.
 * Make sure that the definition below is correct,
 * otherwise all reported times are wrong.
 * To get an idea of what HZ may be, lookup the manpage of times(2)
 *
 * This above comment is dated now. On most systems there is a
 * sysconf(2) call to retrieve such system dependand values.
 * There is no need to depend on compile time fixed values.
 * So, we introduce a variable ClockTick, that contains the
 * number of ticks the clock gets. 
 * This method works for the following types of Unix:
 *	SunOS, SunSolaris, HPUX[9,10], IRIX6, FreeBSD, Linux
 * So it seems reasonable to abandon the old definition of HZ
 * completly.
 */
static  struct tms StartBuffer;
static  int	mSec;
static long ClockTick;

void StartTimer(void)
{
	ClockTick= sysconf(_SC_CLK_TCK);
	times(&StartBuffer);
}

long StopTimer(void)
{
	struct tms StopBuffer;
	long ticks;

	times(&StopBuffer);
	ticks= (StopBuffer.tms_utime+StopBuffer.tms_stime -
		StartBuffer.tms_utime - StartBuffer.tms_stime);
	
#ifdef NODEF
	mSec= (int)((((float)ticks)/ClockTick - (float)(ticks/ClockTick))*1000);
#endif	/* NODEF */
	mSec= ((ticks % ClockTick)*1000)/ClockTick;	/* V3.62  NG */
	return ticks/ClockTick;
}
int MilliSec() {
	return mSec;
}

#endif /* UNIX */

#ifdef VMS                      /* V3.03  TLi */

#       include <time.h>
#       define  HZ      100
		/* also in meiner Doku steht, dass die Zeit in
		 * 10 Tausendsteln Sekunden gemessen wird   TLi
		 */
static  struct tbuffer StartBuffer;

void StartTimer(void)
{
	times(&StartBuffer);
}

long StopTimer(void)
{
	struct tbuffer StopBuffer;

	times(&StopBuffer);
	return (StopBuffer.proc_user_time +
		StopBuffer.proc_system_time -
		StartBuffer.proc_user_time -
		StartBuffer.proc_system_time) / HZ;
}
#endif /* VMS */

#ifdef DOS
#       ifdef __TURBOC__
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
** 	ThisTime= clock();
** }
** 
** long StopTimer(void)
** {
** 	return (clock() - ThisTime)/CLK_TCK;
** }
*/

#       include <time.h>

static time_t ThisTime;

void StartTimer(void)
{
	ThisTime= time((time_t *)0);
}

long StopTimer(void)
{
	return time((time_t *)0) - ThisTime;
}
#       else    /* not __TURBOC__ */

#       ifdef MSC
/* we are using MicroSoft C Compiler */

#include <time.h>

static clock_t ThisTime;

void StartTimer(void)
{
	ThisTime= clock();
}

long StopTimer(void)
{
	return (clock() - ThisTime)/CLK_TCK;
}

#       else /* not MSC */
#       ifdef GCC
	/* we are using GCC */

#       include <time.h>

static time_t ThisTime;

void StartTimer(void)
{
	ThisTime= time((time_t *)0);
}

long StopTimer(void)
{
	return time((time_t *)0) - ThisTime;
}
#       else    /* not GCC */

/* we are using any other C Compiler */

#include <time.h>

static clock_t ThisTime;

void StartTimer(void)
{
	ThisTime= clock();
}

long StopTimer(void)
{
	return (clock() - ThisTime)/CLK_TCK;
}

#endif /* GCC */
#endif /* MSC */
#endif /* __TURBOC__ */
#endif /* DOS */

#ifdef macintosh        /* is always defined on macintosh's  SB */

#       define SEGMIO
#       include "pymac.h"
#       include <Time.h>

#       ifndef NULL
#               define NULL 0
#       endif

#       define size_t unsigned int
#       define CLK_TCK 60

static clock_t ThisTime;

void StartTimer(void)
{
	ThisTime = clock();
}

long StopTimer(void)
{
	return(( clock() - ThisTime )/CLK_TCK);
}
#endif  /* macintosh */

#ifdef ATARI                   /* 2.4 StHoe */

#               include <stddef.h>
#               include <time.h>

static  clock_t ThisTime;

void StartTimer(void)
{
	ThisTime= clock();
}

long StopTimer(void)
{
	return (clock() - ThisTime)/CLK_TCK;
}

#endif  /* ATARI */


#ifdef BS2000                   /* V3.1  NG, RB */

#       include <time.h>

/*
 * We can use clock() here, because in BS2000 C V2.0A
 * this gives the cpu-time consumed since start of program
 */

static  clock_t ThisTime;
void StartTimer(void)
{
	ThisTime= clock();
}

long StopTimer(void)
{
	return (clock() - ThisTime)/CLOCKS_PER_SEC;
}

#endif  /* BS2000 */

#ifdef OS2
#       ifdef GCC
	/* we are using GCC */

#       include <time.h>

static time_t ThisTime;

void StartTimer(void)
{
	ThisTime= time((time_t *)0);
}

long StopTimer(void)
{
	return time((time_t *)0) - ThisTime;
}

#       endif /* GCC */
#endif /* OS2 */

#ifdef C370
#include <time.h>        /* Diese Loesung ist ungenau, da nicht die */
			 /* reine CPU-Zeit, sondern die Differenz   */
static time_t ThisTime;  /* Startzeit - Stopzeit angegeben wird.    */
			 /* Die exakte Laufzeit kann man nur dem    */
void StartTimer(void)    /* JES Job-Log entnehmen.   HD  07/07/93   */
{
	ThisTime= time((time_t *)0);
}

long StopTimer(void)
{
	return time((time_t *)0) - ThisTime;
}
#endif /* C370 */

#ifdef WIN16
#include <time.h>
static  clock_t ThisTime;
void StartTimer(void)
{
        ThisTime= clock();
}
long StopTimer(void)
{
        return (clock() - ThisTime)/CLK_TCK;
}
#endif  /* WIN16 */

#ifdef WIN32
#	if defined(MSC) || defined(__BORLANDC__)
	/* we are using Microsoft C++ compiler
	** or BorlandC 4.something  --  V3.53  TLi
	*/

#       include <time.h>

static  clock_t StartBuffer;
static  int	mSec;

void StartTimer(void)
{
	StartBuffer= clock();
}

long StopTimer(void)
{
	clock_t StopBuffer;
	long ticks;

	StopBuffer= clock();
	ticks= (StopBuffer - StartBuffer);
	
#ifdef NODEF
	mSec= (int)((((float)ticks)/CLOCKS_PER_SEC - (float)(ticks/CLOCKS_PER_SEC))*1000);
#endif	/* NODEF */
	mSec= ((ticks % (long)CLOCKS_PER_SEC)*1000)/(long)CLOCKS_PER_SEC;	/* V3.62  NG */
	return ticks/(long)CLOCKS_PER_SEC;
}
int MilliSec() {
	return mSec;
}

#       endif /* MSC || __BORLANDC__ */
#endif /* WIN32 */
