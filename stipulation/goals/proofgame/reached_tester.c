#include "stipulation/goals/proofgame/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyproof.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "trace.h"

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

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_proofgame_reached_tester_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (ProofIdentical())
    result = slice_solve(slices[si].u.pipe.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
