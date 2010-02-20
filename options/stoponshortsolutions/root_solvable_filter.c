#include "optimisations/stoponshortsolutions/root_solvable_filter.h"
#include "optimisations/stoponshortsolutions/stoponshortsolutions.h"
#include "pypipe.h"
#include "trace.h"

/* Allocate a STStopOnShortSolutionsRootSolvableFilter slice.
 * @return allocated slice
 */
slice_index alloc_stoponshortsolutions_root_solvable_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STStopOnShortSolutionsRootSolvableFilter); 

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean stoponshortsolutions_root_solvable_filter_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  reset_short_solution_found_in_phase();

  result = slice_root_solve(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
