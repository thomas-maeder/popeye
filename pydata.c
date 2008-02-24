			/* PYDATA.C     */
/*************************** MODIFICATIONS *****************************
**
** made compilable with MPW on macintosh-computers.     14.02.91     SB
**
**************************** INFORMATION END **************************/

#include <stdio.h>

#if defined(macintosh)        /* is always defined on macintosh's  SB */
#       define SEGMIO
#       include "pymac.h"
#endif
 
#include "py.h"
#include "pyproc.h"
#define EXTERN
#include "pydata.h"

smallint maxinbox[] = {0,0,0,0,1,2,2,2};
unsigned int boxsize = sizeof maxinbox / sizeof maxinbox[0];

killer_state const null_killer_state = {
  false,
  {
    initsquare,
    initsquare,
    initsquare
  }
};
