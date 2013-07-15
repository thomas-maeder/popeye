#include "pieces/walks/chinese/riders.h"
#include "pieces/walks/riders.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for a chinese rider piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void chinese_rider_generate_moves(square sq_departure,
                                  vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;

  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_hurdle = generate_moves_on_line_segment(sq_departure,
                                                            sq_departure,
                                                            k);
    if (!is_square_blocked(sq_hurdle))
    {
      square const sq_capture = find_end_of_line(sq_hurdle,vec[k]);
      if (piece_belongs_to_opponent(sq_capture))
        add_to_move_generation_stack(sq_departure,sq_capture,sq_capture);
    }
  }
}
