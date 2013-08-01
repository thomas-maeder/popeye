#include "conditions/extinction.h"
#include "pydata.h"
#include "solving/check.h"
#include "solving/observation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_extinction_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_instrument_check_testing(si,STExtinctionCheckTester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean extinction_check_tester_is_in_check(slice_index si, Side side_in_check)
{
  Side const side_checking = advers(side_in_check);
  boolean result = false;

  PieNam p;

  siblingply(side_checking);
  current_move[nbply] = current_move[nbply-1]+1;

  for (p = King; p<PieceCount; ++p)
    if (exist[p] && number_of_pieces[side_in_check][p]==1)
    {
      square const *bnp;
      for (bnp  = boardnum; *bnp; ++bnp)
        if (get_walk_of_piece_on_square(*bnp)==p
            && TSTFLAG(spec[*bnp],side_in_check))
          break;

      move_generation_stack[current_move[nbply]].capture = *bnp;
      if (is_square_observed(&validate_observation))
      {
        result = true;
        break;
      }
    }

  finply();

  return result;
}
