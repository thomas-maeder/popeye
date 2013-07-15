#include "pieces/walks/reflecting_bishop.h"
#include "pieces/walks/angle/angles.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

static void reflecting_bishop_generate_moves_recursive(square orig_departure,
                                                       square in,
                                                       numvec k,
                                                       int x)
{
  int k1;

  square sq_departure= orig_departure;
  square sq_arrival= in+k;

  if (is_square_blocked(sq_arrival))
    return;

  while (is_square_empty(sq_arrival))
  {
    add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k;
  }

  if (piece_belongs_to_opponent(sq_arrival))
    add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
  else if (x && is_square_blocked(sq_arrival))
  {
    sq_arrival-= k;
    k1= 5;
    while (vec[k1]!=k)
      k1++;
    k1*= 2;
    reflecting_bishop_generate_moves_recursive(orig_departure,sq_arrival,angle_vectors[angle_90][k1],x-1);
    k1--;
    reflecting_bishop_generate_moves_recursive(orig_departure,sq_arrival,angle_vectors[angle_90][k1],x-1);
  }
}

/* Generate moves for an reflecting bishop
 * @param sq_departure common departure square of the generated moves
 */
void reflecting_bishop_generate_moves(square sq_departure)
{
  numecoup const save_current_move = current_move[nbply];
  vec_index_type k;
  for (k= vec_bishop_start; k <= vec_bishop_end; k++)
    reflecting_bishop_generate_moves_recursive(sq_departure,sq_departure,vec[k],4);
  remove_duplicate_moves_of_single_piece(save_current_move);
}

/* Generate moves for an reflecting bishop
 * @param sq_departure common departure square of the generated moves
 */
void archbishop_generate_moves(square sq_departure)
{
  numecoup const save_current_move = current_move[nbply];
  vec_index_type k;
  for (k = vec_bishop_start; k<=vec_bishop_end; ++k)
    reflecting_bishop_generate_moves_recursive(sq_departure,sq_departure,vec[k],1);
  if (!NoEdge(sq_departure))
    remove_duplicate_moves_of_single_piece(save_current_move);
}
