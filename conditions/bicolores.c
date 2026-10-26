#include "conditions/bicolores.h"
#include "position/position.h"
#include "solving/ply.h"
#include "solving/check.h"
#include "solving/machinery/solve.h"
#include "solving/pipe.h"

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean bicolores_check_tester_is_in_check(slice_index si, Side side_in_check)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParamListEnd();

  if (pipe_is_in_check_recursive_delegate(si,side_in_check))
    result = true;
  else
  {
    square const sq_king_in_check_square = being_solved.king_square[side_in_check];
    Flags const spec_king_in_check = being_solved.spec[sq_king_in_check_square];

    Side const other_side = advers(side_in_check);
    square const sq_other_king = being_solved.king_square[other_side];
    Flags const spec_other_king = being_solved.spec[sq_other_king];

    trait[nbply] = advers(trait[nbply]);

    being_solved.king_square[side_in_check] = sq_other_king;
    being_solved.spec[sq_king_in_check_square] = spec_other_king;

    being_solved.king_square[other_side] = sq_king_in_check_square;
    being_solved.spec[sq_other_king] = spec_king_in_check;

    result = pipe_is_in_check_recursive_delegate(si,advers(side_in_check));

    being_solved.king_square[side_in_check] = sq_king_in_check_square;
    being_solved.spec[sq_king_in_check_square] = spec_king_in_check;

    being_solved.king_square[other_side] = sq_other_king;
    being_solved.spec[sq_other_king] = spec_other_king;

    trait[nbply] = advers(trait[nbply]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument the solving machinery with Bicolores
 * @param si root slice of the solving machinery
 */
void bicolores_initalise_solving(slice_index si)
{
  solving_instrument_check_testing(si,STBicoloresCheckTester);
}
