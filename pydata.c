			/* PYDATA.C     */
/*************************** MODIFICATIONS *****************************
**
** made compilable with MPW on macintosh-computers.     14.02.91     SB
**
** 2008/02/25 SE   New piece type: Magic  
**
**************************** INFORMATION END **************************/

#include <stdio.h>

#if defined(macintosh)        /* is always defined on macintosh's  SB */
#       define SEGMIO
#       include "platform/unix/mac.h"
#endif
 
#include "py.h"
#include "pyproc.h"
#define EXTERN
#include "pydata.h"

killer_state const null_killer_state = {
  false,
  {
    initsquare,
    initsquare,
    initsquare
  },
  initsquare
};

change_rec* colour_change_stack_limit = &colour_change_stack[colour_change_stack_size];
change_rec* push_colour_change_stack_limit = &push_colour_change_stack[push_colour_change_stack_size];
