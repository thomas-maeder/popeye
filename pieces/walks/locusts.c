#include "pieces/walks/locusts.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate a single Locust capture
 * @param sq_capture capture square
 * @param index_arrival_capturek identifies the vector from capture to arrival square
 */
void generate_locust_capture(square sq_capture,
                             vec_index_type index_arrival_capture)
{
  if (piece_belongs_to_opponent(sq_capture))
  {
    square const sq_arrival= sq_capture+vec[index_arrival_capture];
    if (is_square_empty(sq_arrival))
      push_move_generation_capture_extra(sq_arrival,sq_capture);
  }
}

/* Generate moves for an Locust piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void locust_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;
  for (k= kbeg; k <= kend; k++)
  {
    square const sq_capture = find_end_of_line(curr_generation->departure,vec[k]);
    generate_locust_capture(sq_capture,k);
  }
}
