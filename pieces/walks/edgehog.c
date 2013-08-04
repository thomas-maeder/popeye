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
    curr_generation->arrival = sq_departure+vec[k];
    while (is_square_empty(curr_generation->arrival))
    {
      if (NoEdge(curr_generation->arrival)!=NoEdge(sq_departure))
        push_move();
      curr_generation->arrival += vec[k];
    }

    if (piece_belongs_to_opponent(curr_generation->arrival)
        && NoEdge(curr_generation->arrival)!=NoEdge(sq_departure))
      push_move();
  }
}
