#include "pieces/walks/reflecting_bishop.h"
#include "pieces/walks/angle/angles.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

#include <assert.h>

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
    push_move();
    curr_generation->arrival += dir;
  }

  if (piece_belongs_to_opponent(curr_generation->arrival))
    push_move();
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
  numecoup const save_current_move = current_move[nbply]-1;
  vec_index_type k;
  for (k= vec_bishop_start; k <= vec_bishop_end; k++)
    reflecting_bishop_generate_moves_recursive(curr_generation->departure,vec[k],4);
  remove_duplicate_moves_of_single_piece(save_current_move);
}

/* Generate moves for an reflecting bishop
 */
void archbishop_generate_moves(void)
{
  numecoup const save_current_move = current_move[nbply]-1;
  vec_index_type k;
  for (k = vec_bishop_start; k<=vec_bishop_end; ++k)
    reflecting_bishop_generate_moves_recursive(curr_generation->departure,vec[k],1);
  if (!NoEdge(curr_generation->departure))
    remove_duplicate_moves_of_single_piece(save_current_move);
}
