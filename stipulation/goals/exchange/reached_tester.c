#include "stipulation/goals/exchange/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether an exchange goal has just been reached
 */

/* Allocate a STGoalExchangeReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_exchange_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalExchangeReachedTester);

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
has_solution_type goal_exchange_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;
  square const sq_arrival = move_generation_stack[nbcou].arrival;
  square const sq_diagram = GetDiaRen(spec[sq_arrival]);
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbcou!=nil_coup);

  if (GetDiaRen(spec[sq_diagram])==sq_arrival
      && (just_moved==White ? e[sq_diagram]>=roib : e[sq_diagram]<=roin)
      && sq_diagram!=sq_arrival)
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
has_solution_type goal_exchange_reached_tester_solve(slice_index si)
{
  has_solution_type result;
  square const sq_arrival = move_generation_stack[nbcou].arrival;
  square const sq_diagram = GetDiaRen(spec[sq_arrival]);
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbcou!=nil_coup);

  if (GetDiaRen(spec[sq_diagram])==sq_arrival
      && (just_moved==White ? e[sq_diagram]>=roib : e[sq_diagram]<=roin)
      && sq_diagram!=sq_arrival)
    result = slice_solve(slices[si].u.pipe.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
