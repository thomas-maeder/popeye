#include "pieces/walks/hamster.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

/* Generate moves for an Hamster
 */
void hamster_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle = find_end_of_line(sq_departure,vec[k]);
    if (!is_square_blocked(sq_hurdle))
    {
      curr_generation->arrival = sq_hurdle-vec[k];
      if (curr_generation->arrival!=sq_departure)
        push_move_no_capture();
    }
  }
}

/* hamsters cannot check - no check function
 */

/* Generate moves for a contrahamster
 */
void contrahamster_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle = sq_departure-vec[k];
    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      curr_generation->arrival = sq_departure+vec[k]; /* not allowing null moves */
      while (is_square_empty(curr_generation->arrival))
      {
        push_move_no_capture();
        curr_generation->arrival += vec[k];
      }
     // printf("~ dep: %d dir: %d hur: %d arr: %d\n",sq_departure , vec[k] , sq_hurdle , curr_generation->arrival );
      if (piece_belongs_to_opponent(curr_generation->arrival))
        push_move_regular_capture();
    }
  }
}

boolean contrahamster_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_queen_start;
       interceptable_observation[observation_context].vector_index1<=vec_queen_end;
       interceptable_observation[observation_context].vector_index1++)
  {
    numvec const dir = vec[interceptable_observation[observation_context].vector_index1];
    square const sq_departure = find_end_of_line(sq_target,dir);
    square const sq_hurdle = sq_departure+dir;
   // printf("+ dep: %d dir: %d hur: %d arr: %d\n",sq_departure , dir , sq_hurdle , sq_target );
    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
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
