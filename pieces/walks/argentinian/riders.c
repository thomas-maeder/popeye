#include "pieces/walks/argentinian/riders.h"
#include "pieces/walks/riders.h"
#include "pieces/walks/lions.h"
#include "pieces/walks/hoppers.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

/* Generate moves for an argentinian rider piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void argentinian_rider_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;

  for (k = kbeg; k<=kend; ++k)
  {
    numvec const dir = vec[k];
    curr_generation->arrival = find_end_of_line(curr_generation->departure,dir);

    if (piece_belongs_to_opponent(curr_generation->arrival))
      push_move();

    for (curr_generation->arrival += dir;
         is_square_empty(curr_generation->arrival);
         curr_generation->arrival += dir)
      push_move();
  }
}

boolean senora_check(validator_id evaluate)
{
  return riders_check(vec_queen_start,vec_queen_end, evaluate);
}

boolean faro_check(validator_id evaluate)
{
  return riders_check(vec_rook_start,vec_rook_end, evaluate);
}

boolean loco_check(validator_id evaluate)
{
  return riders_check(vec_bishop_start,vec_bishop_end, evaluate);
}
