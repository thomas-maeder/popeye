#include "stipulation/goals/circuit_by_rebirth/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include <assert.h>

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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
goal_circuit_by_rebirth_reached_tester_attack(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(nbcou!=nil_coup);

  /* goal is only reachable in some fairy conditions */
  result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
