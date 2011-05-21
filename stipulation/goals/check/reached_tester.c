#include "stipulation/goals/check/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a check goal has just been reached
 */

/* Allocate a system of slices that tests whether check has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_check_reached_tester_system(void)
{
  slice_index result;
  slice_index check_tester;
  Goal const goal = { goal_check, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  check_tester = alloc_goal_check_reached_tester_slice(goal_applies_to_starter);
  pipe_link(check_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,check_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STGoalCheckReachedTester slice.
 * @param starter_or_adversary is the starter or its adversary to be in check?
 * @return index of allocated slice
 */
slice_index alloc_goal_check_reached_tester_slice(goal_applies_to_starter_or_adversary starter_or_adversary)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalCheckReachedTester);
  slices[result].u.goal_filter.applies_to_who = starter_or_adversary;

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
has_solution_type goal_check_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;
  Side const in_check = (slices[si].u.goal_filter.applies_to_who
                         ==goal_applies_to_starter
                         ? slices[si].starter
                         : advers(slices[si].starter));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,in_check))
    result = slice_has_solution(slices[si].u.pipe.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
