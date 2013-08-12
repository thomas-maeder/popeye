#include "pieces/walks/riders.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves to the square on a line segment
 * @param sq_base first square of line segment
 * @param k vector index indicating the direction of the line segment
 */
square generate_moves_on_line_segment(square sq_base, vec_index_type k)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_base);
  TraceFunctionParamListEnd();

  curr_generation->arrival = sq_base+vec[k];

  while (is_square_empty(curr_generation->arrival))
  {
    push_move();
    curr_generation->arrival += vec[k];
  }

  TraceFunctionExit(__func__);
  TraceSquare(curr_generation->arrival);
  TraceFunctionResultEnd();
  return curr_generation->arrival;
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
    curr_generation->arrival = generate_moves_on_line_segment(curr_generation->departure,k);
    if (piece_belongs_to_opponent(curr_generation->arrival))
      push_move();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean riders_check(vec_index_type kanf, vec_index_type kend,
                     evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  ++observation_context;

  TraceEnumerator(Side,trait[nbply],"\n");
  for (interceptable_observation[observation_context].vector_index = kanf;
       interceptable_observation[observation_context].vector_index<= kend;
       ++interceptable_observation[observation_context].vector_index)
  {
    square const sq_departure = find_end_of_line(sq_target,vec[interceptable_observation[observation_context].vector_index]);
    if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
