#include "pieces/walks/locusts.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate a single Locust capture
 * @param sq_departure departure square
 * @param sq_capture capture square
 * @param index_arrival_capturek identifies the vector from capture to arrival square
 */
void generate_locust_capture(square sq_departure, square sq_capture,
                             vec_index_type index_arrival_capture)
{
  if (piece_belongs_to_opponent(sq_capture))
  {
    square const sq_arrival= sq_capture+vec[index_arrival_capture];
    if (is_square_empty(sq_arrival))
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_capture);
  }
}

/* Generate moves for an Locust piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void locust_generate_moves(square sq_departure,
                           vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;
  for (k= kbeg; k <= kend; k++)
  {
    square const sq_capture = find_end_of_line(sq_departure,vec[k]);
    generate_locust_capture(sq_departure,sq_capture,k);
  }
}
