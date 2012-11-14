#include "position/position.h"
#include "py.h"
#include "debugging/trace.h"

#define ENUMERATION_TYPENAME Side
#define ENUMERATORS \
  ENUMERATOR(White), \
    ENUMERATOR(Black), \
                       \
    ENUMERATOR(nr_sides), \
    ASSIGNED_ENUMERATOR(no_side = nr_sides)

#define ENUMERATION_MAKESTRINGS

#include "utilities/enumeration.h"

echiquier e;
Flags spec[maxsquare+4];
square king_square[nr_sides];
boolean areColorsSwapped;
boolean isBoardReflected;

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

  pos->king_square[White] = square_e1;
  pos->king_square[Black] = square_e8;

  for (i = 0; i <derbla; ++i)
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

/* Swap the sides of all the pieces */
void swap_sides(void)
{
  square const *bnp;
  square const save_white_king_square = king_square[White];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  king_square[White] = king_square[Black]==initsquare ? initsquare : king_square[Black];
  king_square[Black] = save_white_king_square==initsquare ? initsquare : save_white_king_square;

  for (bnp = boardnum; *bnp; bnp++)
    if (!TSTFLAG(spec[*bnp],Neutral) && e[*bnp]!=vide)
    {
      e[*bnp] = -e[*bnp];
      spec[*bnp]^= BIT(White)+BIT(Black);
    }

  areColorsSwapped = !areColorsSwapped;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Reflect the position at the horizontal central line */
void reflect_position(void)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  king_square[White] = king_square[White]==initsquare ? initsquare : transformSquare(king_square[White],mirra1a8);
  king_square[Black] = king_square[Black]==initsquare ? initsquare : transformSquare(king_square[Black],mirra1a8);

  for (bnp = boardnum; *bnp < (square_a1+square_h8)/2; bnp++)
  {
    square const sq_reflected = transformSquare(*bnp,mirra1a8);

    piece const p = e[sq_reflected];
    Flags const sp = spec[sq_reflected];

    e[sq_reflected] = e[*bnp];
    spec[sq_reflected] = spec[*bnp];

    e[*bnp] = p;
    spec[*bnp] = sp;
  }

  isBoardReflected = !isBoardReflected;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
