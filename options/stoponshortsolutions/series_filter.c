#include "optimisations/stoponshortsolutions/series_filter.h"
#include "optimisations/stoponshortsolutions/stoponshortsolutions.h"
#include "stipulation/series_play/play.h"
#include "stipulation/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STStopOnShortSolutionsSeriesFilter slice.
 * @param length full length 
 * @param length minimum length 
 * @return allocated slice
 */
slice_index
alloc_stoponshortsolutions_series_filter(stip_length_type length,
                                         stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_branch(STStopOnShortSolutionsSeriesFilter,length,min_length); 

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 *         n-1 the previous move has solved the next slice
 */
stip_length_type
stoponshortsolutions_series_filter_solve_in_n(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (has_short_solution_been_found_in_phase())
    result = n+1;
  else if (series_solve_in_n(slices[si].u.pipe.next,n)<=n)
  {
    if (n<slices[si].u.branch.length)
      short_solution_found();
    result = n;
  }
  else
    result = n+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 *         n-1 the previous move has solved the next slice
 */
stip_length_type
stoponshortsolutions_series_filter_has_solution_in_n(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = series_has_solution_in_n(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
