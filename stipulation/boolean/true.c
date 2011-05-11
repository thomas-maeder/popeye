#include "stipulation/boolean/true.h"
#include "trace.h"

/* This module provides functionality dealing with leaf slices
 */

/* Allocate a STTrue slice.
 * @return index of allocated slice
 */
slice_index alloc_true_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_slice(STTrue);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type true_has_solution(slice_index si)
{
  has_solution_type const result = has_solution;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type true_solve(slice_index si)
{
  has_solution_type const result = has_solution;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
