#include "pieces/walks/rabbit.h"
#include "pieces/walks/riders.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

/* Generate moves for a Rabbit
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
        curr_generation->arrival = generate_moves_on_line_segment(sq_arrival,k);
        if (piece_belongs_to_opponent(curr_generation->arrival))
          push_move_regular_capture();
      }
    }
  }
}
