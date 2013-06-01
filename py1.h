/******************** MODIFICATIONS to py1.h **************************
**
** Date       Who  What
**
** 2001       TLi  Original
**
**************************** End of List ******************************/

#if !defined(PY1_INCLUDED)
#define PY1_INCLUDED

#include "pyproc.h"
#include "pydata.h"
#include "conditions/haunted_chess.h"

boolean find_next_friend_in_chain(square i,
                                  PieNam pfr,
                                  PieNam p,
                                  evalfunction_t *evaluate);

void InitCheckDir(void);
void InitBoard(void);
void InitOpt(void);
void InitAlways(void);

#endif /*PY1_INCLUDED*/
