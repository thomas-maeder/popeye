#include "stipulation/goals/enpassant/reached_tester.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a enpassant goal has just been reached
 */

/* Allocate a system of slices that tests whether enpassant has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_enpassant_reached_tester_system(void)
{
  slice_index result;
  slice_index enpassant_tester;
  Goal const goal = { goal_ep, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  enpassant_tester = alloc_pipe(STGoalEnpassantReachedTester);
  pipe_link(enpassant_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,enpassant_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type goal_enpassant_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(nbply!=nil_ply);

  if (move_effect_journal[capture].type==move_effect_piece_removal
      && move_effect_journal[capture].reason==move_effect_reason_ep_capture)
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
