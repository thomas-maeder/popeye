#include "conditions/annan.h"
#include "position/position.h"

#include <assert.h>
#include <stdlib.h>

annan_type_type annan_type;

boolean annanises(Side side, square rear, square front)
{
  if (TSTFLAG(spec[rear],side))
    switch(annan_type)
    {
      case annan_type_A:
        return true;

      case annan_type_B:
        return rear!=king_square[side];

      case annan_type_C:
        return front!=king_square[side];

      case annan_type_D:
        return rear!=king_square[side] && front!=king_square[side];

      default:
        assert(0);
        return true;
    }
  else
    return false;
}

boolean annan_is_square_attacked(Side side_attacking,
                                 square sq_target,
                                 evalfunction_t *evaluate)
{
  numvec const dir_annaniser = side_attacking==White ? dir_down : dir_up;
  square annan_sq[nr_squares_on_board];
  piece annan_p[nr_squares_on_board];
  int annan_cnt = 0;
  boolean result;
  unsigned int i;
  square square_a = side_attacking==White ? square_a8 : square_a1;

  for (i = nr_rows_on_board-1; i>0; --i, square_a += dir_annaniser)
  {
    square pos_annanised = square_a;
    unsigned int j;
    for (j = nr_files_on_board; j>0; --j, ++pos_annanised)
    {
      square const pos_annaniser = pos_annanised+dir_annaniser;
      if (TSTFLAG(spec[pos_annanised],side_attacking)
          && annanises(side_attacking,pos_annaniser,pos_annanised))
      {
        annan_sq[annan_cnt] = pos_annanised;
        annan_p[annan_cnt] = abs(e[pos_annanised]);
        ++annan_cnt;
        replace_piece(pos_annanised,abs(e[pos_annaniser]));
      }
    }
  }

  result= is_a_square_attacked(side_attacking,sq_target,evaluate);

  while (annan_cnt--)
    replace_piece(annan_sq[annan_cnt],annan_p[annan_cnt]);

  return result;
}
