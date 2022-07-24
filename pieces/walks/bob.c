#include "pieces/walks/bob.h"
#include "pieces/walks/riders.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/fork.h"
#include "debugging/trace.h"

/* Generate moves for an Bob
 */
void bob_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle1 = find_end_of_line(curr_generation->departure,vec[k]);
    if (!is_square_blocked(sq_hurdle1))
    {
      square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[k]);
      if (!is_square_blocked(sq_hurdle2))
      {
        square const sq_hurdle3 = find_end_of_line(sq_hurdle2,vec[k]);
        if (!is_square_blocked(sq_hurdle3))
        {
          square const sq_arrival = find_end_of_line(sq_hurdle3,vec[k]);
          if (!is_square_blocked(sq_arrival))
          {
            curr_generation->arrival = generate_moves_on_line_segment(sq_arrival,k);
            if (piece_belongs_to_opponent(curr_generation->arrival))
              push_move_regular_capture();
          }
        }
      }
    }
  }
}

boolean bob_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  /* 4 hurdle lion */
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_queen_end;
       interceptable_observation[observation_context].vector_index1>=vec_queen_start;
       interceptable_observation[observation_context].vector_index1--)
  {
    square const sq_hurdle1 = find_end_of_line(sq_target,vec[interceptable_observation[observation_context].vector_index1]);
    if (!is_square_empty(sq_hurdle1) && !is_square_blocked(sq_hurdle1))
    {
      square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[interceptable_observation[observation_context].vector_index1]);
      if (!is_square_blocked(sq_hurdle2))
      {
        square const sq_hurdle3 = find_end_of_line(sq_hurdle2,vec[interceptable_observation[observation_context].vector_index1]);
        if (!is_square_blocked(sq_hurdle3))
        {
          square const sq_hurdle4 = find_end_of_line(sq_hurdle3,vec[interceptable_observation[observation_context].vector_index1]);
          if (!is_square_blocked(sq_hurdle4))
          {
            square const sq_departure = find_end_of_line(sq_hurdle4,vec[interceptable_observation[observation_context].vector_index1]);
            if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
            {
              result = true;
              break;
            }
          }
        }
      }
    }
  }

  --observation_context;

  return result;
}
