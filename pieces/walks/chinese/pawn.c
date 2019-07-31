#include "pieces/walks/chinese/pawn.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "solving/fork.h"
#include "debugging/trace.h"

/* Generate moves for a chinese pawn
 */
void chinese_pawn_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  Side const opponent = advers(trait[nbply]);
  boolean const past_river = (trait[nbply]==White
                              ? 2*sq_departure>square_h8+square_a1
                              : 2*sq_departure<square_h8+square_a1);

  {
    int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    curr_generation->arrival = sq_departure+dir_forward;
    if (is_square_empty(curr_generation->arrival))
      push_move_no_capture();
    else if (TSTFLAG(being_solved.spec[curr_generation->arrival],opponent))
      push_move_regular_capture();
  }

  if (past_river)
  {
    {
      curr_generation->arrival = sq_departure+dir_right;
      if (is_square_empty(curr_generation->arrival))
        push_move_no_capture();
      else if (TSTFLAG(being_solved.spec[curr_generation->arrival],opponent))
        push_move_regular_capture();
    }
    {
      curr_generation->arrival = sq_departure+dir_left;
      if (is_square_empty(curr_generation->arrival))
        push_move_no_capture();
      else if (TSTFLAG(being_solved.spec[curr_generation->arrival],opponent))
        push_move_regular_capture();
    }
  }
}

boolean chinese_pawn_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  square sq_departure;
  numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;

  sq_departure= sq_target+dir_backward;
  if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
    return true;

  /* chinese pawns can capture side-ways if standing on the half of
   * the board farther away from their camp's base line (i.e. if
   * black, on the lower half, if white on the upper half) */
  if ((sq_target*2<(square_h8+square_a1)) == (trait[nbply]==Black))
  {
    sq_departure= sq_target+dir_right;
    if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      return true;

    sq_departure= sq_target+dir_left;
    if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}
