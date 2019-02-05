#include "pieces/walks/cat.h"
#include "pieces/walks/leapers.h"
#include "solving/move_generator.h"
#include "solving/fork.h"
#include "debugging/trace.h"

/* Generate moves for a cat
 */
void cat_generate_moves(void)
{
  /* generate moves of a CAT */
  vec_index_type k;

  for (k= vec_knight_start; k<=vec_knight_end; k++)
  {
    curr_generation->arrival = curr_generation->departure+vec[k];
    if (piece_belongs_to_opponent(curr_generation->arrival))
      push_move_regular_capture();
    else
    {
      while (is_square_empty(curr_generation->arrival))
      {
        push_move_no_capture();
        curr_generation->arrival += cat_vectors[k];
      }

      if (piece_belongs_to_opponent(curr_generation->arrival))
        push_move_regular_capture();
    }
  }
}

boolean cat_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  if (leapers_check(vec_knight_start,vec_knight_end,evaluate))
    return true;
  else
  {
    vec_index_type  k;
    for (k = vec_dabbaba_start; k<=vec_dabbaba_end; k++)
    {
      square middle_square= sq_target+vec[k];
      while (is_square_empty(middle_square))
      {
        {
          square const sq_departure= middle_square+cat_vectors[k-60];
          if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
            return true;
        }

        {
          square const sq_departure= middle_square+cat_vectors[k-56];
          if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
            return true;
        }

        middle_square += vec[k];
      }
    }

    return false;
  }
}
