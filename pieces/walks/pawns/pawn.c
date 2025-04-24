#include "pieces/walks/pawns/pawn.h"
#include "pieces/pieces.h"
#include "conditions/conditions.h"
#include "conditions/circe/circe.h"
#include "pieces/walks/pawns/pawns.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/move_generator.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

unsigned int pawn_get_no_capture_length(Side side, square sq_departure)
{
  unsigned int result;
  SquareFlags const base_square = WhBaseSq+side;
  SquareFlags const doublestep_square = WhPawnDoublestepSq+side;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (TSTFLAG(sq_spec(sq_departure),base_square))
  {
    if (CondFlag[einstein] || CondFlag[antieinstein] || CondFlag[reveinstein])
      result = 3;
    else if ((circe_variant.determine_rebirth_square==circe_determine_rebirth_square_equipollents
              && circe_variant.relevant_capture==circe_relevant_capture_lastmove)
             || CondFlag[normalp]
             || circe_variant.determine_rebirth_square==circe_determine_rebirth_square_cage
             || get_walk_of_piece_on_square(sq_departure)==Orphan /* we are generating for a pawned Orphan! */
             || TSTFLAG(sq_spec(sq_departure),Wormhole))
      result = 1;
    else
      result = 0;
  }
  else if (TSTFLAG(sq_spec(sq_departure),doublestep_square))
    result = 2;
  else
    result = 1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void pawn_generate_moves(void)
{
  unsigned int no_capture_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  no_capture_length = pawn_get_no_capture_length(trait[nbply],
                                                 curr_generation->departure);

  if (no_capture_length>0)
  {
    int const dir_vertical = trait[nbply]==White ? dir_up : dir_down;

    pawns_generate_capture_move(dir_vertical+dir_left);
    pawns_generate_capture_move(dir_vertical+dir_right);
    pawns_generate_nocapture_moves(dir_vertical,no_capture_length);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Does any pawn deliver check?
 * @param sq_departure departure square of king capaturing move
 * @param sq_arrival arrival square of king capaturing move
 * @return if any pawn delivers check
 */
boolean pawn_test_check(square sq_departure,
                        square sq_arrival,
                        validator_id evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  result = EVALUATE_OBSERVATION(evaluate,sq_departure,sq_arrival);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean pawn_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(sq_target);
  TraceEOL();

  if (TSTFLAG(sq_spec(sq_target),capturable) || observing_walk[nbply]==Orphan || observing_walk[nbply]>=Hunter0)
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    numvec const dir_forward_right = dir_forward+dir_right;
    numvec const dir_forward_left = dir_forward+dir_left;

    if (pawn_test_check(sq_target-dir_forward_right,sq_target,evaluate))
      result = true;
    else if (pawn_test_check(sq_target-dir_forward_left,sq_target,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_right,&pawn_test_check,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_left,&pawn_test_check,evaluate))
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
