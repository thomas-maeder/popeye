#include "pieces/walks/pawns/pawn.h"
#include "pydata.h"
#include "pieces/walks/pawns/pawns.h"
#include "solving/move_generator.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "debugging/trace.h"

#include <assert.h>

unsigned int pawn_get_no_capture_length(Side side, square sq_departure)
{
  unsigned int result;
  SquareFlags const base_square = WhBaseSq+side;
  SquareFlags const doublestep_square = WhPawnDoublestepSq+side;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (TSTFLAG(sq_spec[sq_departure],base_square))
  {
    if (CondFlag[einstein])
      result = 3;
    else if (anyparrain
             || CondFlag[normalp]
             || CondFlag[circecage]
             || get_walk_of_piece_on_square(sq_departure)==Orphan /* we are generating for a pawned Orphan! */
             || TSTFLAG(sq_spec[sq_departure],Wormhole))
      result = 1;
    else
      result = 0;
  }
  else if (TSTFLAG(sq_spec[sq_departure],doublestep_square))
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
                        evalfunction_t *evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  result = (TSTFLAG(spec[sq_departure],trait[nbply])
            && INVOKE_EVAL(evaluate,sq_departure,sq_arrival));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
