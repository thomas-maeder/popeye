/******************** MODIFICATIONS to pysignal.c **************************

**
** Date       Who  What
** 
** 1996/11/25 ElB  Original
**
** 1997/12/24 ElB  Added SignalBased changes of HashRateLevel.
** 
** 1999/01/14 NG   Small change for DJGPP compatibility (__GO32__)
** 
** 1999/01/17 NG   Better solution for option  MaxTime using
**                 alarm signal under UNIX
**                 and sleeping thread under WIN32.
**                 Not supported under DOS.
** 
** 2000/01/21 TLi  ReDrawBoard modified
**
**************************** End of List ******************************/ 
 
#if defined(UNIX) && defined(SIGNALS)

#include <stdio.h>
#include <signal.h>
#include "py.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"

#if defined(HASHRATE)
extern int HashRateLevel;

void IncHashRateLevel(int sig) {
    HashRateLevel++;
    StdString("  ");
    PrintTime(TimeString);
    logIntArg(HashRateLevel);
    Message(IncrementHashRateLevel);
    HashStats(0, "\n");
    signal(SIGUSR1, IncHashRateLevel);
}

void DecHashRateLevel(int sig) {
    if (HashRateLevel > 0)
	HashRateLevel--;
    StdString("  ");
    PrintTime(TimeString);
    logIntArg(HashRateLevel);
    Message(DecrementHashRateLevel);
    HashStats(0, "\n");
    signal(SIGUSR2, DecHashRateLevel);
}
#endif /*HASHRATE*/


void GotSignal(int sig) {
    sprintf(GlobalStr, GetMsgString(Abort), sig, MakeTimeString());
    StdString(GlobalStr);
    exit(1);
}

/* this initialisation is valid only for Unix.
 * I do not know whether there are other handling
 * possiblilities. May be on Mac's and Amigas.
 * or windows?
 */
static int SignalToCatch[] = {
        SIGHUP,
        SIGINT,
        SIGQUIT,
        SIGTERM,
#if !defined(HPUX) && !defined(__GO32__)
/* not supported by HP-UX */
/* not supported by DJGPP */
        SIGVTALRM,
        SIGXCPU,
        SIGXFSZ,
#endif /* HPUX, __GO32__ */
        0
};

void ReDrawBoard(int sig) {
    /* I did this, to see more accurately
     * what position popeye is working on.
     *				ElB, 1998-01-11. 
     */
    /* If a position can be reached by 1000's
     * of move sequences than the position is of almost
     * no value. The history is more important.
     *                          TLi, 2000-02-21.
     */
    ply pl;

    WritePosition();

    /* and write (some information about) the
     * sequences of moves that lead to this position.
     *                           V3.62  TLi
     */
    for (pl= 3; pl < nbply; pl++) {
      WritePiece(pjoue[pl-1]);
      WriteSquare(cd[repere[pl]]);
        StdChar('-');
      WriteSquare(ca[repere[pl]]);
      if (norm_prom[pl-1]) {
        StdChar('=');
        WritePiece(norm_prom[pl-1]);
      }
      StdString("   ");
    }
    StdChar('\n');

    signal(SIGHUP, ReDrawBoard);
}

void UNIXSolvingTimeOver(int sig) {
/*
 * To stop the calculation of a problem
 * after a given amount of time is over.
 *				 V3.54  NG
 */
	FlagTimeOut= true;
	signal(SIGALRM,  UNIXSolvingTimeOver);
}

void pyInitSignal(void) {
    int i;

    i=0;
    while (SignalToCatch[i]) {
        signal(SignalToCatch[i], GotSignal);
        i++;
    }

    /* this initialisation is very simple. 
     * this code would be much more robust, when
     * some information about the signals were
     * available without knowing the semantics of
     * the Unix-Signal numbers.
     * At least the maximum signal-number should be
     * defined and for what signals the handling can
     * be redefined
     */

#if defined(HASHRATE)
    signal(SIGUSR1, IncHashRateLevel);
    signal(SIGUSR2, DecHashRateLevel);
#endif /*HASHRATE*/
    signal(SIGALRM,  UNIXSolvingTimeOver);
    signal(SIGHUP,  ReDrawBoard);
}

#endif /*UNIX,SIGNALS*/

#ifdef WIN32			/* V3.54  NG */

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include "py.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"

void WIN32SolvingTimeOver(int *WaitTime) {
/*
 * This function is used by a WIN32-thread to wake up
 * the thread after WaitTime seconds.
 */
int mythread= GlobalThreadCounter;

	/* sleep under WIN32 seems to use milliseconds ... */
	_sleep(1000 * (*WaitTime));
	/* To avoid that a not "used" thread stops */
	/* Popeye during "his" timeout             */
	/* GlobalThreadCounter is increased in py6.c */
	/* every time a new problem is to be solved. */
	if (mythread == GlobalThreadCounter) { 
		FlagTimeOut= true;
		FlagTimerInUse= false;
	}
_endthread();
}		

#endif	/* WIN32 */
