#include "stipulation/goals/proofgame/reached_tester.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "pyproof.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a proof game goal has just been reached
 */

/* Allocate a system of slices that tests whether proofgame has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_proofgame_reached_tester_system(void)
{
  slice_index result;
  slice_index proofgame_tester;
  Goal const goal = { goal_proofgame, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  proofgame_tester = alloc_pipe(STGoalProofgameReachedTester);
  pipe_link(proofgame_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,proofgame_tester);

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
stip_length_type goal_proofgame_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (ProofIdentical())
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
