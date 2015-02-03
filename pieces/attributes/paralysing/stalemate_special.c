#include "pieces/attributes/paralysing/stalemate_special.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "solving/check.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* This module provides slice type STPiecesParalysingStalemateSpecial
 */

/* Allocate a STPiecesParalysingStalemateSpecial slice.
 * @param starter_or_adversary is the starter stalemated or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_paralysing_stalemate_special_slice(goal_applies_to_starter_or_adversary starter_or_adversary)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",starter_or_adversary);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesParalysingStalemateSpecial);
  SLICE_U(result).goal_filter.applies_to_who = starter_or_adversary;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void paralysing_stalemate_special_solve(slice_index si)
{
  goal_applies_to_starter_or_adversary const
    applies_to_who = SLICE_U(si).goal_filter.applies_to_who;
  Side const starter = SLICE_STARTER(si);
  Side const stalemated = (applies_to_who==goal_applies_to_starter
                           ? starter
                           : advers(starter));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* only flag selfcheck if the side that has just moved is not the one to be
   * stalemated (i.e. if the stipulation is not auto-stalemate) */
  if (applies_to_who==goal_applies_to_starter && is_in_check(advers(starter)))
    solve_result = previous_move_is_illegal;
  else
    pipe_this_move_solves_if(si,suffocated_by_paralysis(stalemated));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
