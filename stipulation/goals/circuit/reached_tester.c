#include "stipulation/goals/circuit/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether a circuit goal has just been reached
 */

/* Allocate a system of slices that tests whether circuit has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_circuit_reached_tester_system(void)
{
  slice_index result;
  slice_index circuit_tester;
  Goal const goal = { goal_circuit, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  circuit_tester = alloc_pipe(STGoalCircuitReachedTester);
  pipe_link(circuit_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,circuit_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
