#include "pieces/walks/bouncer.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Bouncer
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void bouncer_generate_moves(square sq_departure,
                            vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type  k;
  for (k = kend; k>=kbeg; k--)
  {
    square const bounce_where = find_end_of_line(sq_departure,vec[k]);
    square const bounce_to = 2*sq_departure-bounce_where;

    square sq_arrival = sq_departure-vec[k];
    while (sq_arrival!=bounce_to && is_square_empty(sq_arrival))
      sq_arrival -= vec[k];

    if (sq_arrival==bounce_to
        && (is_square_empty(sq_arrival)
            || piece_belongs_to_opponent(sq_arrival)))
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
  }
}
