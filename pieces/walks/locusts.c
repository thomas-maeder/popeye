#include "pieces/walks/locusts.h"
#include "pieces/walks/hoppers.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

/* Generate a single Locust capture
 * @param sq_capture capture square
 * @param dir_arrival_capture vector from capture to arrival square
 */
void generate_locust_capture(square sq_capture,
                             numvec dir_arrival_capture)
{
  if (piece_belongs_to_opponent(sq_capture))
  {
    curr_generation->arrival = sq_capture+dir_arrival_capture;
    if (is_square_empty(curr_generation->arrival))
    {
      push_move_capture_extra(sq_capture);
      hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].idx_dir = 0;
      hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_capture;
    }
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
    numvec const dir = vec[k];
    square const sq_capture = find_end_of_line(curr_generation->departure,dir);
    generate_locust_capture(sq_capture,dir);
  }
}

static boolean locusts_check(vec_index_type kanf, vec_index_type kend,
                             evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = kanf;
       interceptable_observation[observation_context].vector_index1<=kend;
       interceptable_observation[observation_context].vector_index1++)
  {
    square const sq_arrival = sq_target-vec[interceptable_observation[observation_context].vector_index1];
    if (is_square_empty(sq_arrival))
    {
      square const sq_departure = find_end_of_line(sq_target,vec[interceptable_observation[observation_context].vector_index1]);
      if (INVOKE_EVAL(evaluate,sq_departure,sq_arrival))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}

boolean locust_check(evalfunction_t *evaluate)
{
  return locusts_check(vec_queen_start, vec_queen_end, evaluate);
}

boolean rooklocust_check(evalfunction_t *evaluate)
{
  return locusts_check(vec_rook_start, vec_rook_end, evaluate);
}

boolean bishoplocust_check(evalfunction_t *evaluate)
{
  return locusts_check(vec_bishop_start, vec_bishop_end, evaluate);
}

boolean nightlocust_check(evalfunction_t *evaluate)
{
  return locusts_check(vec_knight_start, vec_knight_end, evaluate);
}
