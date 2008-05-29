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
**		   alarm signal under UNIX
**		   and sleeping thread under WIN32.
**		   Not supported under DOS.
**
** 2000/01/21 TLi  ReDrawBoard modified
**
** 2003/05/18 NG   new option: beep    (if solution encountered)
**
**************************** End of List ******************************/

#if defined(__unix) 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	/* to import prototype of 'sleep' NG  */

void BeepOnSolution(int NumOfBeeps)
{
  while (NumOfBeeps > 0)
  {
	fprintf(stderr, "\a");
    fflush(stderr);
	NumOfBeeps--;
  }
  sleep(1);
}

#endif /* __unix */


#if defined(_WIN32)

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include "py.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"

void BeepOnSolution(int NumOfBeeps) {
  while (NumOfBeeps > 0) {
	fprintf(stderr, "\a");
    fflush(stderr);
	NumOfBeeps--;
  }
  /* sleep under WIN32 seems to use milliseconds ... */
  _sleep(500);
}

#endif	/* _WIN32 */


#if defined(__APPLE__) && defined(__MACH__)

#include <stdio.h>
#include <unistd.h>

void BeepOnSolution(int NumOfBeeps) {
  while (NumOfBeeps > 0) {
	fprintf(stderr, "\a");
    fflush(stderr);
	NumOfBeeps--;
  }
  
  sleep(1);
}

#endif
