			/* PYDATA.C     */
/*************************** MODIFICATIONS *****************************
**
** made compilable with MPW on macintosh-computers.     14.02.91     SB
**
** 2008/02/25 SE   New piece type: Magic
**
**************************** INFORMATION END **************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if defined(macintosh)        /* is always defined on macintosh's  SB */
#       define SEGMIO
#       include "platform/unix/mac.h"
#endif

#include "py.h"
#define EXTERN
#include "pydata.h"
