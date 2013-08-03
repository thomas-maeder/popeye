#include "pieces/walks/hamster.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Hamster
 */
void hamster_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle = find_end_of_line(sq_departure,vec[k]);
    if (!is_square_blocked(sq_hurdle))
    {
      square const sq_arrival = sq_hurdle-vec[k];
      if (sq_arrival!=sq_departure)
        push_move_generation(sq_arrival);
    }
  }
}
