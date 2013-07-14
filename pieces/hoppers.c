#include "pieces/hoppers.h"
#include "pydata.h"
#include "pyproc.h"

void hoppers_generate_moves(square sq_departure,
                            vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;
  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_hurdle = find_end_of_line(sq_departure,vec[k]);

    if (!is_square_blocked(sq_hurdle))
    {
      square const sq_arrival = sq_hurdle+vec[k];
      if (piece_belongs_to_opponent(sq_arrival)
          || is_square_empty(sq_arrival))
      {
        add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
        move_generation_stack[current_move[nbply]].auxiliary.hopper.vec_index = k;
        move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = sq_hurdle;
      }
    }
  }
}
