#include "pieces/walks/pawns/berolina.h"
#include "pieces/walks/pawns/pawns.h"
#include "pieces/walks/pawns/pawn.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/move_generator.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

/* Generate moves for a Berolina pawn
 */
void berolina_pawn_generate_moves(void)
{
  unsigned int const no_capture_length = pawn_get_no_capture_length(trait[nbply],
                                                                    curr_generation->departure);

  if (no_capture_length>0)
  {
    int const dir_forward = trait[nbply]==White ? dir_up : dir_down;

    pawns_generate_capture_move(dir_forward);
    pawns_generate_nocapture_moves(dir_forward+dir_left,no_capture_length);
    pawns_generate_nocapture_moves(dir_forward+dir_right,no_capture_length);
  }
}

boolean berolina_pawn_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;

  if (TSTFLAG(sq_spec(sq_target),capturable) || observing_walk[nbply]==Orphan || observing_walk[nbply]>=Hunter0)
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;

    if (pawn_test_check(sq_target-dir_forward,sq_target,evaluate))
      return true;
    if (en_passant_test_check(dir_forward,&pawn_test_check,evaluate))
      return true;
  }

  return false;
}
