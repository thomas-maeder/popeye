#include "pieces/walks/edgehog.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Edgehog
 */
void edgehog_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  vec_index_type k;

  for (k= vec_queen_end; k >=vec_queen_start; k--)
  {
    square sq_arrival = sq_departure+vec[k];
    while (is_square_empty(sq_arrival))
    {
      if (NoEdge(sq_arrival)!=NoEdge(sq_departure))
        push_move_generation(sq_arrival);
      sq_arrival+= vec[k];
    }

    if (piece_belongs_to_opponent(sq_arrival)
        && NoEdge(sq_arrival)!=NoEdge(sq_departure))
      push_move_generation(sq_arrival);
  }
}
