#include "pieces/walks/cardinal.h"
#include "pieces/walks/angle/angles.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "solving/fork.h"
#include "debugging/trace.h"

static void cardinal_generate_moves_recursive(square in, numvec k, int x)
{
  int k1;
  curr_generation->arrival = in+k;

  while (is_square_empty(curr_generation->arrival))
  {
    push_move_no_capture();
    curr_generation->arrival += k;
  }

  if (piece_belongs_to_opponent(curr_generation->arrival))
    push_move_regular_capture();
  else if (x && is_square_blocked(curr_generation->arrival))
  {
    for (k1= 1; k1<=4; k1++)
      if (!is_square_blocked(curr_generation->arrival+vec[k1]))
        break;

    if (k1<=4)
    {
      curr_generation->arrival += vec[k1];
      if (piece_belongs_to_opponent(curr_generation->arrival))
        push_move_regular_capture();
      else if (is_square_empty(curr_generation->arrival))
      {
        push_move_no_capture();
        k1= 5;
        while (vec[k1]!=k)
          k1++;
        k1*= 2;
        if (is_square_blocked(curr_generation->arrival+angle_vectors[angle_90][k1]))
          k1--;

        cardinal_generate_moves_recursive(curr_generation->arrival,angle_vectors[angle_90][k1],x-1);
      }
    }
  }
}

/* Generate moves for a Cardinal
 */
void cardinal_generate_moves(void)
{
  vec_index_type k;
  for (k= vec_bishop_start; k <= vec_bishop_end; k++)
    cardinal_generate_moves_recursive(curr_generation->departure,vec[k],1);
}

static boolean cardinal_check_recursive(square intermediate_square,
                                        numvec k,
                                        int    x,
                                        validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  square sq_departure = find_end_of_line(intermediate_square,k);

  if (x && is_square_blocked(sq_departure))
  {
    numvec k1;
    for (k1 = 1; k1<=4; k1++)
      if (!is_square_blocked(sq_departure+vec[k1]))
        break;

    if (k1<=4)
    {
      sq_departure += vec[k1];
      if (is_square_empty(sq_departure))
      {
        k1= 5;
        while (vec[k1]!=k)
          k1++;
        k1*= 2;
        if (is_square_blocked(sq_departure+angle_vectors[angle_90][k1]))
          k1--;
        if (cardinal_check_recursive(sq_departure,
                     angle_vectors[angle_90][k1],
                     x-1,
                     evaluate))
          return true;
      }
      else if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
        return true;
    }
  }
  else if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target ))
    return true;

  return false;
}

boolean cardinal_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (cardinal_check_recursive(sq_target, vec[k], 1, evaluate)) {
      return true;
    }
  }
  return false;
}
