#include "pieces/walks/riders.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves to the square on a line segment
 * @param sq_base first square of line segment
 * @param k vector index indicating the direction of the line segment
 */
square generate_moves_on_line_segment(square sq_base, vec_index_type k)
{
  square sq_arrival = sq_base+vec[k];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_base);
  TraceFunctionParamListEnd();

  while (is_square_empty(sq_arrival))
  {
    push_move_generation(sq_arrival);
    sq_arrival += vec[k];
  }

  TraceFunctionExit(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionResultEnd();
  return sq_arrival;
}

/* Generate moves for a rider piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void rider_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  /* generate rider moves from vec[kbeg] to vec[kend] */
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kbeg);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_arrival = generate_moves_on_line_segment(curr_generation->departure,k);
    if (piece_belongs_to_opponent(sq_arrival))
      push_move_generation(sq_arrival);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
