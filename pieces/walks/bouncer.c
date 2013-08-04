#include "pieces/walks/bouncer.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Bouncer
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void bouncer_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  square const sq_departure = curr_generation->departure;
  vec_index_type  k;
  for (k = kend; k>=kbeg; k--)
  {
    square const bounce_where = find_end_of_line(sq_departure,vec[k]);
    square const bounce_to = 2*sq_departure-bounce_where;

    curr_generation->arrival = sq_departure-vec[k];
    while (curr_generation->arrival!=bounce_to
           && is_square_empty(curr_generation->arrival))
      curr_generation->arrival -= vec[k];

    if (curr_generation->arrival==bounce_to
        && (is_square_empty(curr_generation->arrival)
            || piece_belongs_to_opponent(curr_generation->arrival)))
      push_move();
  }
}
