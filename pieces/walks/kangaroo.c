#include "pieces/walks/kangaroo.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Kangaroo
 */
void kangaroo_generate_moves(void)
{
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle1 = find_end_of_line(curr_generation->departure,vec[k]);
    if (!is_square_blocked(sq_hurdle1))
    {
      square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[k]);
      if (!is_square_blocked(sq_hurdle2))
      {
        curr_generation->arrival = sq_hurdle2+vec[k];
        if (is_square_empty(curr_generation->arrival)
            || piece_belongs_to_opponent(curr_generation->arrival))
          push_move();
      }
    }
  }
}

boolean kangoucheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_queen_end;
       interceptable_observation[observation_context].vector_index1>=vec_queen_start;
       interceptable_observation[observation_context].vector_index1--)
  {
    square const sq_hurdle1 = sq_target+vec[interceptable_observation[observation_context].vector_index1];
    if (!is_square_empty(sq_hurdle1) && !is_square_blocked(sq_hurdle1))
    {
      square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[interceptable_observation[observation_context].vector_index1]);
      if (!is_square_blocked(sq_hurdle2))
      {
        square const sq_departure = find_end_of_line(sq_hurdle2,vec[interceptable_observation[observation_context].vector_index1]);
        if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
        {
          result = true;
          break;
        }
      }
    }
  }

  --observation_context;

  return result;
}

/* Generate moves for an Kangaroo Lion
 */
void kangaroo_lion_generate_moves(void)
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
        curr_generation->arrival = sq_hurdle2+vec[k];
        while (is_square_empty(curr_generation->arrival))
        {
          push_move();
          curr_generation->arrival += vec[k];
        }
        if (piece_belongs_to_opponent(curr_generation->arrival))
          push_move();
      }
    }
  }
}

boolean kanglioncheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
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
        square const sq_departure = find_end_of_line(sq_hurdle2,vec[interceptable_observation[observation_context].vector_index1]);
        if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
        {
          result = true;
          break;
        }
      }
    }
  }

  --observation_context;

  return result;
}
