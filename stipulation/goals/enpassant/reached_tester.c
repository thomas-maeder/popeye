#include "stipulation/goals/enpassant/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a enpassant goal has just been reached
 */

/* Allocate a system of slices that tests whether enpassant has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_enpassant_reached_tester_system(void)
{
  slice_index result;
  slice_index enpassant_tester;
  Goal const goal = { goal_ep, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  enpassant_tester = alloc_pipe(STGoalEnpassantReachedTester);
  pipe_link(enpassant_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,enpassant_tester);

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
has_solution_type goal_enpassant_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbply!=nil_ply);

  if (move_generation_stack[nbcou].arrival!=move_generation_stack[nbcou].capture
      && is_pawn(pjoue[nbply]))
    result = slice_has_solution(slices[si].u.pipe.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
