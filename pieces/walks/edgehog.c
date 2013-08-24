#include "pieces/walks/edgehog.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

/* Generate moves for an Edgehog
 */
void edgehog_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  vec_index_type k;

  for (k= vec_queen_end; k >=vec_queen_start; k--)
  {
    curr_generation->arrival = sq_departure+vec[k];
    while (is_square_empty(curr_generation->arrival))
    {
      if (NoEdge(curr_generation->arrival)!=NoEdge(sq_departure))
        push_move();
      curr_generation->arrival += vec[k];
    }

    if (piece_belongs_to_opponent(curr_generation->arrival)
        && NoEdge(curr_generation->arrival)!=NoEdge(sq_departure))
      push_move();
  }
}

boolean edgehog_check(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_queen_end;
      interceptable_observation[observation_context].vector_index1>=vec_queen_start;
      interceptable_observation[observation_context].vector_index1--)
  {
    square const sq_departure = find_end_of_line(sq_target,vec[interceptable_observation[observation_context].vector_index1]);
    if (NoEdge(sq_target)!=NoEdge(sq_departure)
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  return result;
}
