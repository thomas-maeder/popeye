#include "stipulation/goals/circuit/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "trace.h"

#include <assert.h>

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

  assert(nbcou!=nil_coup);

  if (GetDiaRen(spec[sq_arrival])==sq_arrival)
    result = slice_has_solution(slices[si].u.pipe.next);
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

  assert(nbcou!=nil_coup);

  if (GetDiaRen(spec[sq_arrival])==sq_arrival)
    result = slice_solve(slices[si].u.pipe.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
