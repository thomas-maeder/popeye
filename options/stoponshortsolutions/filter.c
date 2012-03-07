#include "options/stoponshortsolutions/filter.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "stipulation/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STStopOnShortSolutionsFilter slice.
 * @param length full length
 * @param length minimum length
 * @return allocated slice
 */
slice_index alloc_stoponshortsolutions_filter(stip_length_type length,
                                              stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_branch(STStopOnShortSolutionsFilter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move leading to the current position has
 *                           turned out to be illegal
 *            n   solution found
 *            n+2 no solution found
 */
stip_length_type stoponshortsolutions_help(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (has_short_solution_been_found_in_phase())
    result = n+2;
  else
  {
    result = help(slices[si].u.pipe.next,n);
    if (result<=n && n<slices[si].u.branch.length)
      short_solution_found();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
