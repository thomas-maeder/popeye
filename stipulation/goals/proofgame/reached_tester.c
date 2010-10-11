#include "stipulation/goals/proofgame/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyproof.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a proof game goal has just been reached
 */

/* Allocate a STGoalProofgameReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_proofgame_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalProofgameReachedTester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_proofgame_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (ProofIdentical())
  {
    if (echecc(nbply,advers(slices[si].starter)))
      result = opponent_self_check;
    else
      result = slice_has_solution(slices[si].u.pipe.next);
  }
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
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
  {
    if (echecc(nbply,advers(slices[si].starter)))
      result = opponent_self_check;
    else
      result = slice_solve(slices[si].u.pipe.next);
  }
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
