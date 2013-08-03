#include "pieces/walks/rabbit.h"
#include "pieces/walks/riders.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Rabbit
 */
void rabbit_generate_moves(void)
{
  vec_index_type k;

  for (k= vec_queen_end; k >=vec_queen_start; k--)
  {
    square const sq_hurdle = find_end_of_line(curr_generation->departure,vec[k]);
    if (!is_square_blocked(sq_hurdle))
    {
      square sq_arrival = find_end_of_line(sq_hurdle,vec[k]);
      if (!is_square_blocked(sq_arrival))
      {
        square const sq_capture = generate_moves_on_line_segment(sq_arrival,k);
        if (piece_belongs_to_opponent(sq_capture))
          push_move_generation(sq_capture);
      }
    }
  }
}
