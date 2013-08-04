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

  for (k= vec_knight_start; k<=vec_knight_end; k++)
  {
    curr_generation->arrival = curr_generation->departure+vec[k];
    if (piece_belongs_to_opponent(curr_generation->arrival))
      push_move();
    else
    {
      while (is_square_empty(curr_generation->arrival))
      {
        push_move();
        curr_generation->arrival += cat_vectors[k];
      }

      if (piece_belongs_to_opponent(curr_generation->arrival))
        push_move();
    }
  }
}
