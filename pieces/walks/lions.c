#include "pieces/walks/lions.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

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
        curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
        push_move();
        curr_generation->arrival += vec[k];
      }

      if (piece_belongs_to_opponent(curr_generation->arrival))
      {
        curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
        push_move();
      }
    }
  }

  curr_generation->auxiliary.hopper.sq_hurdle = initsquare;
}

boolean lions_check(vec_index_type kanf, vec_index_type kend,
                    evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index = kanf;
       interceptable_observation[observation_context].vector_index<=kend;
       interceptable_observation[observation_context].vector_index++)
  {
    square const sq_hurdle = find_end_of_line(sq_target,vec[interceptable_observation[observation_context].vector_index]);

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_hurdle,vec[interceptable_observation[observation_context].vector_index]);

      if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}

boolean nightriderlioncheck(evalfunction_t *evaluate)
{
    return lions_check(vec_knight_start, vec_knight_end, evaluate);
}

boolean lioncheck(evalfunction_t *evaluate)
{
    return lions_check(vec_queen_start,vec_queen_end, evaluate);
}

boolean t_lioncheck(evalfunction_t *evaluate)
{
    return lions_check(vec_rook_start,vec_rook_end, evaluate);
}

boolean f_lioncheck(evalfunction_t *evaluate)
{
    return lions_check(vec_bishop_start,vec_bishop_end, evaluate);
}
