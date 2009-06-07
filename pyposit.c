#include "pyposit.h"
#include "trace.h"
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

position game_array;

static void initialise_game_array(void)
{
  int i;
  square const *bnp;

  game_array.rb = square_e1;
  game_array.rn = square_e8;

  for (i = roib; i <= derbla; ++i)
  {
    game_array.nr_piece[-dernoi+i] = 0;
    game_array.nr_piece[-dernoi-i] = 0;
  }

  /* TODO avoid duplication with InitBoard()
   */
  for (i = 0; i<maxsquare; ++i)
  {
    game_array.board[i] = obs;
    game_array.spec[i] = BorderSpec;
  }

  for (bnp = boardnum; *bnp; bnp++)
  {
    game_array.board[*bnp] = vide;
    CLEARFL(game_array.spec[*bnp]);
  }

  for (i = 0; i<nr_squares_on_board; ++i)
  {
    piece const p = PAS[i];
    square const square_i = boardnum[i];
    game_array.board[square_i] = p;
    ++game_array.nr_piece[-dernoi+p];
    if (p>=roib)
      SETFLAG(game_array.spec[square_i],White);
    else if (p<=roin)
      SETFLAG(game_array.spec[square_i],Black);
  }

  game_array.inum = 0;
}

void position_initialise_module(void)
{
  initialise_game_array();
}
