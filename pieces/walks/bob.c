#include "pieces/walks/bob.h"
#include "pieces/walks/riders.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Bob
 */
void bob_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle1 = find_end_of_line(curr_generation->departure,vec[k]);
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
            curr_generation->arrival = generate_moves_on_line_segment(sq_arrival,k);
            if (piece_belongs_to_opponent(curr_generation->arrival))
              push_move();
          }
        }
      }
    }
  }
}
