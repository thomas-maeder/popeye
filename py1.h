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

boolean fffriendcheck(
    square        i,
    piece         pfr,
    piece         p,
    evalfunction_t *evaluate);

void InitCheckDir(void);
void InitBoard(void);
void InitOpt(void);
void InitAlways(void);

typedef struct
{
  Flags       spec[nr_squares_on_board];
  piece       e[nr_squares_on_board];
  int inum1;
  imarr       isquare;
  square      im0, rn_sic, rb_sic;
  ghosts_type ghosts;
  ghost_index_type nr_ghosts;
} stored_position_type;

void    ResetPosition(stored_position_type const *store);
void    StorePosition(stored_position_type *store);

#endif /*PY1_INCLUDED*/
