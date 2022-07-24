#include "pieces/walks/chinese/leapers.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

/* Generate moves for a chinese leaper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void chinese_leaper_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;

  for (k = kbeg; k<=kend; ++k)
  {
    curr_generation->arrival = curr_generation->departure + vec[k];

    if (is_square_empty(curr_generation->arrival))
      push_move_no_capture();
    else if (!is_square_blocked(curr_generation->arrival))
    {
      curr_generation->arrival += vec[k];
      if (piece_belongs_to_opponent(curr_generation->arrival))
        push_move_regular_capture();
    }
  }
}
