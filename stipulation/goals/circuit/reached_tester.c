#include "stipulation/goals/circuit/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a circuit goal has just been reached
 */

/* Allocate a STGoalCircuitReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_circuit_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalCircuitReachedTester);
  slices[result].u.goal_reached_tester.goal.type = goal_circuit;

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
has_solution_type goal_circuit_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;
  square const sq_arrival = move_generation_stack[nbcou].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (GetDiaRen(spec[sq_arrival])==sq_arrival)
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
has_solution_type goal_circuit_reached_tester_solve(slice_index si)
{
  has_solution_type result;
  square const sq_arrival = move_generation_stack[nbcou].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (GetDiaRen(spec[sq_arrival])==sq_arrival)
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
