#include "stipulation/goals/target/reached_tester.h"
#include "pystip.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a goal has just been reached
 */

/* Allocate a system of slices that tests whether a target square has been
 * reached
 * @return index of entry slice
 */
slice_index alloc_goal_target_reached_tester_system(square target)
{
  slice_index result;
  slice_index target_tester;
  Goal const goal = { goal_target, target };

  TraceFunctionEntry(__func__);
  TraceSquare(target);
  TraceFunctionParamListEnd();

  target_tester = alloc_pipe(STGoalTargetReachedTester);
  slices[target_tester].u.goal_handler.goal.type = goal_target;
  slices[target_tester].u.goal_handler.goal.target = target;
  pipe_link(target_tester,alloc_true_slice());

  result = alloc_goal_reached_tester_slice(goal,target_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type goal_target_reached_tester_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const target = slices[si].u.goal_handler.goal.target;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(nbcou!=nil_coup);

  if (move_generation_stack[nbcou].arrival==target)
    result = attack(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
