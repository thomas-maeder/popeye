#include "stipulation/goals/circuit_by_rebirth/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether a circuit (by rebirth) goal has just been reached
 */

/* Allocate a system of slices that tests whether circuit_by_rebirth has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_circuit_by_rebirth_reached_tester_system(void)
{
  slice_index result;
  slice_index circuit_by_rebirth;
  Goal const goal = { goal_circuit_by_rebirth, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  circuit_by_rebirth = alloc_pipe(STGoalCircuitByRebirthReachedTester);
  pipe_link(circuit_by_rebirth,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,circuit_by_rebirth);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
