#include "stipulation/goals/king_capture/reached_tester.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

/* This module provides functionality dealing with slices that detect
 * whether an any goal has just been reached
 */


/* Allocate a system of slices that tests whether king capture has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_king_capture_reached_tester_system(void)
{
  slice_index result;
  slice_index any_tester;
  Goal const goal = { goal_any, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  any_tester = alloc_pipe(STGoalKingCaptureReachedTester);
  pipe_link(any_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,any_tester);

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
stip_length_type goal_king_capture_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,slices[si].starter,"\n");
  if (king_square[slices[si].starter]==nullsquare)
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
