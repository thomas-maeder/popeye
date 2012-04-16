#include "stipulation/goals/capture/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a capture goal has just been reached
 */

/* Allocate a system of slices that tests whether capture has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_capture_reached_tester_system(void)
{
  slice_index result;
  slice_index capture_tester;
  Goal const goal = { goal_capture, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  capture_tester = alloc_pipe(STGoalCaptureReachedTester);
  pipe_link(capture_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,capture_tester);

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
stip_length_type goal_capture_reached_tester_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(nbply!=nil_ply);

  if (pprise[nbply]!=vide)
    result = attack(slices[si].u.pipe.next,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
