#include "pieces/walks/bob.h"
#include "pieces/walks/riders.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Bob
 * @param sq_departure common departure square of the generated moves
 */
void bob_generate_moves(square sq_departure)
{
  vec_index_type k;

  for (k = vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle1 = find_end_of_line(sq_departure,vec[k]);
    if (!is_square_blocked(sq_hurdle1))
    {
      square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[k]);
      if (!is_square_blocked(sq_hurdle2))
      {
        square const sq_hurdle3 = find_end_of_line(sq_hurdle2,vec[k]);
        if (!is_square_blocked(sq_hurdle3))
        {
          square const sq_arrival = find_end_of_line(sq_hurdle3,vec[k]);
          if (!is_square_blocked(sq_arrival))
          {
            square const sq_capture = generate_moves_on_line_segment(sq_departure,
                                                                     sq_arrival,
                                                                     k);
            if (piece_belongs_to_opponent(sq_capture))
              add_to_move_generation_stack(sq_departure,sq_capture,sq_capture);
          }
        }
      }
    }
  }
}
