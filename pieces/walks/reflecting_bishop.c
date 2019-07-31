#include "pieces/walks/reflecting_bishop.h"
#include "pieces/walks/angle/angles.h"
#include "solving/move_generator.h"
#include "solving/fork.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "position/position.h"

#include "debugging/assert.h"

static vec_index_type find_vec_index(numvec dir)
{
  vec_index_type result = vec_bishop_start;

  while (vec[result]!=dir)
    result++;

  assert(result<=vec_bishop_end);

  return result;
}

static void reflecting_bishop_generate_moves_recursive(square in, numvec dir, unsigned int nr_remaining_reflections)
{
  curr_generation->arrival = in+dir;

  if (is_square_blocked(curr_generation->arrival))
    return;

  while (is_square_empty(curr_generation->arrival))
  {
    push_move_no_capture();
    curr_generation->arrival += dir;
  }

  if (piece_belongs_to_opponent(curr_generation->arrival))
    push_move_regular_capture();
  else if (nr_remaining_reflections>0 && is_square_blocked(curr_generation->arrival))
  {
    square const sq_reflection = curr_generation->arrival-dir;

    vec_index_type const dir_index = find_vec_index(dir);
    vec_index_type const dir_reflected_index = dir_index*2;

    reflecting_bishop_generate_moves_recursive(sq_reflection,
                                               angle_vectors[angle_90][dir_reflected_index],
                                               nr_remaining_reflections-1);
    reflecting_bishop_generate_moves_recursive(sq_reflection,
                                               angle_vectors[angle_90][dir_reflected_index-1],
                                               nr_remaining_reflections-1);
  }
}

/* Generate moves for an reflecting bishop
 */
void reflecting_bishop_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  vec_index_type k;
  for (k= vec_bishop_start; k <= vec_bishop_end; k++)
    reflecting_bishop_generate_moves_recursive(curr_generation->departure,vec[k],4);
  remove_duplicate_moves_of_single_piece(save_current_move);
}

static boolean reflecting_bishop_check_recursive(square intermediate_square,
                                                 numvec k,
                                                 int x,
                                                 validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  if (is_square_blocked(intermediate_square+k))
    return false;
  else
  {
    numvec k1;
    square const sq_reflection = find_end_of_line(intermediate_square,k);
    piece_walk_type const p1 = get_walk_of_piece_on_square(sq_reflection);

    if (x && p1==Invalid)
    {
      square const sq_departure = sq_reflection-k;

      k1= 5;
      while (vec[k1]!=k)
        k1++;

      k1 *= 2;
      if (reflecting_bishop_check_recursive(sq_departure,
                   angle_vectors[angle_90][k1],
                   x-1,
                   evaluate))

        return true;

      k1--;
      if (reflecting_bishop_check_recursive(sq_departure,
                   angle_vectors[angle_90][k1],
                   x-1,
                   evaluate))
        return true;
    }
    else if (EVALUATE_OBSERVATION(evaluate,sq_reflection,sq_target))
      return true;

    return false;
  }
}

boolean reflecting_bishop_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (reflecting_bishop_check_recursive(sq_target, vec[k], 4, evaluate)) {
      return true;
    }
  }
  return false;
}

/* Generate moves for an reflecting bishop
 */
void archbishop_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  vec_index_type k;
  for (k = vec_bishop_start; k<=vec_bishop_end; ++k)
    reflecting_bishop_generate_moves_recursive(curr_generation->departure,vec[k],1);
  if (!NoEdge(curr_generation->departure))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean archbishop_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (reflecting_bishop_check_recursive(sq_target, vec[k], 1, evaluate)) {
      return true;
    }
  }
  return false;
}
