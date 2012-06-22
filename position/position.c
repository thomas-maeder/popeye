#include "position/position.h"

#define ENUMERATION_TYPENAME Side
#define ENUMERATORS \
  ENUMERATOR(White), \
    ENUMERATOR(Black), \
                       \
    ENUMERATOR(nr_sides), \
    ASSIGNED_ENUMERATOR(no_side = nr_sides)

#define ENUMERATION_MAKESTRINGS

#include "utilities/enumeration.h"

/* This is the InitialGameArray */
piece const PAS[nr_squares_on_board] = {
  tb,   cb,   fb,   db, roib,   fb,   cb,   tb,
  pb,   pb,   pb,   pb,   pb,   pb,   pb,   pb,
  vide, vide, vide, vide, vide, vide, vide, vide,
  vide, vide, vide, vide, vide, vide, vide, vide,
  vide, vide, vide, vide, vide, vide, vide, vide,
  vide, vide, vide, vide, vide, vide, vide, vide,
  pn,   pn,   pn,   pn,   pn,   pn,   pn,   pn,
  tn,   cn,   fn,   dn, roin,   fn,   cn,   tn
};

void initialise_game_array(position *pos)
{
  int i;
  square const *bnp;

  pos->rb = square_e1;
  pos->rn = square_e8;

  for (i = 0; i <= derbla; ++i)
  {
    nr_piece(*pos)[i] = 0;
    nr_piece(*pos)[-i] = 0;
  }

  /* TODO avoid duplication with InitBoard()
   */
  for (i = 0; i<maxsquare+4; ++i)
  {
    pos->board[i] = obs;
    pos->spec[i] = BorderSpec;
  }

  for (bnp = boardnum; *bnp; bnp++)
  {
    pos->board[*bnp] = vide;
    CLEARFL(pos->spec[*bnp]);
  }

  for (i = 0; i<nr_squares_on_board; ++i)
  {
    piece const p = PAS[i];
    square const square_i = boardnum[i];
    pos->board[square_i] = p;
    ++nr_piece(*pos)[p];
    if (p>=roib)
      SETFLAG(pos->spec[square_i],White);
    else if (p<=roin)
      SETFLAG(pos->spec[square_i],Black);
  }

  pos->inum = 0;
  for (i = 0; i<maxinum; ++i)
    pos->isquare[i] = initsquare;
}
