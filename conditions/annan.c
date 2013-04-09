#include "conditions/annan.h"
#include "position/position.h"

#include <assert.h>

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

boolean annan_is_black_king_square_attacked(evalfunction_t *evaluate)
{
  square annan_sq[nr_squares_on_board];
  piece annan_p[nr_squares_on_board];
  int annan_cnt= 0;
  boolean ret;

  square i,j,z,z1;
  z= square_h8;
  for (i= nr_rows_on_board-1; i > 0; i--, z-= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z--) {
      z1= z-onerow;
      if (e[z] > obs && annanises(White,z1,z))
      {
        annan_sq[annan_cnt]= z;
        annan_p[annan_cnt]= e[z];
        ++annan_cnt;
        e[z]=e[z1];
      }
    }
  ret= is_black_king_square_attacked(evaluate);

  while (annan_cnt--)
    e[annan_sq[annan_cnt]]= annan_p[annan_cnt];

  return ret;
}

boolean annan_is_white_king_square_attacked(evalfunction_t *evaluate)
{
  square annan_sq[nr_squares_on_board];
  piece annan_p[nr_squares_on_board];
  int annan_cnt= 0;
  boolean ret;

  square i,j,z,z1;
  z= square_a1;
  for (i= nr_rows_on_board-1; i > 0; i--, z+= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z++) {
      z1= z+onerow;
      if (e[z] < vide && annanises(Black,z1,z))
      {
        annan_sq[annan_cnt]= z;
        annan_p[annan_cnt]= e[z];
        ++annan_cnt;
        e[z]=e[z1];
      }
    }
  ret= is_white_king_square_attacked(evaluate);

  while (annan_cnt--)
    e[annan_sq[annan_cnt]]= annan_p[annan_cnt];

  return ret;
}
