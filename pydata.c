			/* PYDATA.C     */
/*************************** MODIFICATIONS *****************************
**
** made compilable with MPW on macintosh-computers.     14.02.91     SB
**
**************************** INFORMATION END **************************/

#include <stdio.h>	/* V3.1A  NG */

#ifdef macintosh        /* is always defined on macintosh's  SB */
#       define SEGMIO
#       include "pymac.h"
#endif
 
#include "py.h"
#include "pyproc.h"             /* V2.60  */
#define EXTERN
#include "pydata.h"

smallint maxinbox[] = {0,0,0,0,1,2,2,2}; /* V3.71 TM */
unsigned int boxsize = sizeof maxinbox / sizeof maxinbox[0]; /* V3.71 TM */
