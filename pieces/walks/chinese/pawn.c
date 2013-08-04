#include "pieces/walks/chinese/pawn.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for a chinese pawn
 */
void chinese_pawn_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  Side const opponent = advers(trait[nbply]);
  boolean const past_river = (trait[nbply]==White
                              ? 2*sq_departure>square_h8+square_a1
                              : 2*sq_departure<square_h8+square_a1);

  {
    int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    curr_generation->arrival = sq_departure+dir_forward;
    if (is_square_empty(curr_generation->arrival)
        || TSTFLAG(spec[curr_generation->arrival],opponent))
      push_move();
  }

  if (past_river)
  {
    {
      curr_generation->arrival = sq_departure+dir_right;
      if (is_square_empty(curr_generation->arrival)
          || TSTFLAG(spec[curr_generation->arrival],opponent))
        push_move();
    }
    {
      curr_generation->arrival = sq_departure+dir_left;
      if (is_square_empty(curr_generation->arrival) || TSTFLAG(spec[curr_generation->arrival],opponent))
        push_move();
    }
  }
}
