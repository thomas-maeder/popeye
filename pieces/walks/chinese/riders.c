#include "pieces/walks/chinese/riders.h"
#include "pieces/walks/riders.h"
#include "pieces/walks/lions.h"
#include "pieces/walks/hoppers.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

/* Generate moves for a chinese rider piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void chinese_rider_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;

  for (k = kbeg; k<=kend; ++k)
  {
    numecoup const base = current_move[nbply];
    square const sq_hurdle = generate_moves_on_line_segment(curr_generation->departure,k);

    /* avoid accidentally "inheriting" the hurdle from some previous move */
    hoppers_clear_hurdles(base);

    if (!is_square_blocked(sq_hurdle))
    {
      curr_generation->arrival = find_end_of_line(sq_hurdle,vec[k]);
      if (piece_belongs_to_opponent(curr_generation->arrival))
        hoppers_push_capture(k,sq_hurdle);
    }
  }
}

boolean leo_check(validator_id evaluate)
{
  return lions_check(vec_queen_start,vec_queen_end, evaluate);
}

boolean pao_check(validator_id evaluate)
{
  return lions_check(vec_rook_start,vec_rook_end, evaluate);
}

boolean vao_check(validator_id evaluate)
{
  return lions_check(vec_bishop_start,vec_bishop_end, evaluate);
}

boolean nao_check(validator_id evaluate)
{
  return lions_check(vec_knight_start,vec_knight_end, evaluate);
}
