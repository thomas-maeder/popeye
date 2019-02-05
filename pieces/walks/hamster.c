#include "pieces/walks/hamster.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

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
      curr_generation->arrival = sq_hurdle-vec[k];
      if (curr_generation->arrival!=sq_departure)
        push_move_no_capture();
    }
  }
}
