#include "conditions/anticirce/target_square_filter.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STAnticirceTargetSquareFilter
 */

/* Allocate a STAnticirceTargetSquareFilter slice.
 * @param target target square to be reached
 * @return index of allocated slice
 */
slice_index alloc_anticirce_target_square_filter_slice(square target)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STAnticirceTargetSquareFilter);
  slices[result].u.goal_target_reached_tester.target = target;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Is the piece that has just moved removed from the target square because it is
 * a Kamikaze piece?
 * @param si identifies filter slice
 * @return true iff the piece is removed
 */
static boolean is_mover_removed_from_target(slice_index si)
{
  boolean result;
  square const target = slices[si].u.goal_target_reached_tester.target;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (e[target]==vide)
    result = true;
  else if (sq_rebirth_capturing[nbply]==target)
    result = false;
  else
    result = true;


  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type anticirce_target_square_filter_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_mover_removed_from_target(si))
    result = has_no_solution;
  else
    result = slice_solve(slices[si].u.goal_target_reached_tester.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
