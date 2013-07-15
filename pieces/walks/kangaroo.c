#include "pieces/walks/kangaroo.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Kangaroo
 * @param sq_departure common departure square of the generated moves
 */
void kangaroo_generate_moves(square sq_departure)
{
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle1 = find_end_of_line(sq_departure,vec[k]);
    if (!is_square_blocked(sq_hurdle1))
    {
      square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[k]);
      if (!is_square_blocked(sq_hurdle2))
      {
        square const sq_arrival = sq_hurdle2+vec[k];
        if (is_square_empty(sq_arrival)
            || piece_belongs_to_opponent(sq_arrival))
          add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

/* Generate moves for an Kangaroo Lion
 * @param sq_departure common departure square of the generated moves
 */
void kangaroo_lion_generate_moves(square sq_departure)
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
        square sq_arrival = sq_hurdle2+vec[k];
        while (is_square_empty(sq_arrival))
        {
          add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
          sq_arrival += vec[k];
        }
        if (piece_belongs_to_opponent(sq_arrival))
          add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}
