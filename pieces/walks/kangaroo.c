#include "pieces/walks/kangaroo.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Kangaroo
 */
void kangaroo_generate_moves(void)
{
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle1 = find_end_of_line(curr_generation->departure,vec[k]);
    if (!is_square_blocked(sq_hurdle1))
    {
      square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[k]);
      if (!is_square_blocked(sq_hurdle2))
      {
        square const sq_arrival = sq_hurdle2+vec[k];
        if (is_square_empty(sq_arrival)
            || piece_belongs_to_opponent(sq_arrival))
          push_move_generation(sq_arrival);
      }
    }
  }
}

/* Generate moves for an Kangaroo Lion
 */
void kangaroo_lion_generate_moves(void)
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
        square sq_arrival = sq_hurdle2+vec[k];
        while (is_square_empty(sq_arrival))
        {
          push_move_generation(sq_arrival);
          sq_arrival += vec[k];
        }
        if (piece_belongs_to_opponent(sq_arrival))
          push_move_generation(sq_arrival);
      }
    }
  }
}
