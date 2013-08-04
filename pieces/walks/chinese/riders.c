#include "pieces/walks/chinese/riders.h"
#include "pieces/walks/riders.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for a chinese rider piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void chinese_rider_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;

  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_hurdle = generate_moves_on_line_segment(curr_generation->departure,k);
    if (!is_square_blocked(sq_hurdle))
    {
      curr_generation->arrival = find_end_of_line(sq_hurdle,vec[k]);
      if (piece_belongs_to_opponent(curr_generation->arrival))
        push_move();
    }
  }
}
