#include "pieces/walks/lions.h"
#include "pieces/walks/hoppers.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/fork.h"
#include "debugging/trace.h"

/* Generate moves for a lion piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void lions_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;
  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_hurdle = find_end_of_line(curr_generation->departure,vec[k]);

    if (!is_square_blocked(sq_hurdle))
    {
      curr_generation->arrival = sq_hurdle+vec[k];
      while (is_square_empty(curr_generation->arrival))
      {
        hoppers_push_move(k,sq_hurdle);
        curr_generation->arrival += vec[k];
      }

      if (piece_belongs_to_opponent(curr_generation->arrival))
        hoppers_push_capture(k,sq_hurdle);
    }
  }
}

boolean lions_check(vec_index_type kanf, vec_index_type kend,
                    validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = kanf;
       interceptable_observation[observation_context].vector_index1<=kend;
       interceptable_observation[observation_context].vector_index1++)
  {
    square const sq_hurdle = find_end_of_line(sq_target,vec[interceptable_observation[observation_context].vector_index1]);

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_hurdle,vec[interceptable_observation[observation_context].vector_index1]);

      if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}

boolean lion_check(validator_id evaluate)
{
  return lions_check(vec_queen_start,vec_queen_end, evaluate);
}

boolean rooklion_check(validator_id evaluate)
{
  return lions_check(vec_rook_start,vec_rook_end, evaluate);
}

boolean bishoplion_check(validator_id evaluate)
{
  return lions_check(vec_bishop_start,vec_bishop_end, evaluate);
}

boolean nightriderlion_check(validator_id evaluate)
{
  return lions_check(vec_knight_start, vec_knight_end, evaluate);
}
