#include "pieces/walks/bouncer.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/fork.h"
#include "debugging/trace.h"

/* Generate moves for an Bouncer
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void bouncer_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  square const sq_departure = curr_generation->departure;
  vec_index_type  k;
  for (k = kend; k>=kbeg; k--)
  {
    square const bounce_where = find_end_of_line(sq_departure,vec[k]);
    square const bounce_to = 2*sq_departure-bounce_where;

    curr_generation->arrival = sq_departure-vec[k];
    while (curr_generation->arrival!=bounce_to
           && is_square_empty(curr_generation->arrival))
      curr_generation->arrival -= vec[k];

    if (curr_generation->arrival==bounce_to)
    {
      if (is_square_empty(curr_generation->arrival))
        push_move_no_capture();
      else if (piece_belongs_to_opponent(curr_generation->arrival))
        push_move_regular_capture();
    }
  }
}

static boolean bouncerfamilycheck(vec_index_type kbeg, vec_index_type kend,
                                  validator_id evaluate)
{
  boolean result = false;
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = kend;
       interceptable_observation[observation_context].vector_index1>=kbeg;
       interceptable_observation[observation_context].vector_index1--)
  {
    numvec const dir = vec[interceptable_observation[observation_context].vector_index1];
    square const sq_departure = find_end_of_line(sq_target,dir);
    square const sq_hurdle = find_end_of_line(sq_departure,dir);
    if (sq_departure-sq_target==sq_hurdle-sq_departure
        && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  return result;
}

boolean bouncer_check(validator_id evaluate)
{
  return bouncerfamilycheck(vec_queen_start,vec_queen_end, evaluate);
}

boolean rookbouncer_check(validator_id evaluate)
{
  return bouncerfamilycheck(vec_rook_start,vec_rook_end, evaluate);
}

boolean bishopbouncer_check(validator_id evaluate)
{
  return bouncerfamilycheck(vec_bishop_start,vec_bishop_end, evaluate);
}
