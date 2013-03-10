#include "stipulation/goals/target/reached_tester.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
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
stip_length_type goal_target_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const target = slices[si].u.goal_handler.goal.target;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(current_move[nbply]!=nil_coup);

  if (move_generation_stack[current_move[nbply]].arrival==target)
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
