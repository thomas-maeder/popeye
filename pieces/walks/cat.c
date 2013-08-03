#include "pieces/walks/cat.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for a cat
 */
void cat_generate_moves(void)
{
  /* generate moves of a CAT */
  vec_index_type k;

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    square sq_arrival = curr_generation->departure+vec[k];
    if (piece_belongs_to_opponent(sq_arrival))
      push_move_generation(sq_arrival);
    else
    {
      while (is_square_empty(sq_arrival))
      {
        push_move_generation(sq_arrival);
        sq_arrival+= cat_vectors[k];
      }

      if (piece_belongs_to_opponent(sq_arrival))
        push_move_generation(sq_arrival);
    }
  }
}
