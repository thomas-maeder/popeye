#include "pieces/walks/edgehog.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Edgehog
 * @param sq_departure common departure square of the generated moves
 */
void edgehog_generate_moves(square sq_departure)
{
  vec_index_type k;

  for (k= vec_queen_end; k >=vec_queen_start; k--)
  {
    square sq_arrival= sq_departure+vec[k];
    while (is_square_empty(sq_arrival))
    {
      if (NoEdge(sq_arrival)!=NoEdge(sq_departure))
        add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= vec[k];
    }

    if (piece_belongs_to_opponent(sq_arrival)
        && NoEdge(sq_arrival)!=NoEdge(sq_departure))
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
  }
}
