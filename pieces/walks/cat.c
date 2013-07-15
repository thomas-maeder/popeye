#include "pieces/walks/cat.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for a rider piece
 * @param sq_departure common departure square of the generated moves
 */
void cat_generate_moves(square sq_departure)
{
  /* generate moves of a CAT */
  vec_index_type k;

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    square sq_arrival = sq_departure+vec[k];
    if (piece_belongs_to_opponent(sq_arrival))
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
    else
    {
      while (is_square_empty(sq_arrival))
      {
        add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
        sq_arrival+= cat_vectors[k];
      }

      if (piece_belongs_to_opponent(sq_arrival))
        add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
    }
  }
}
