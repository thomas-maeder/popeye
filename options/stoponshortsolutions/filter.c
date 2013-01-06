#include "options/stoponshortsolutions/filter.h"
#include "stipulation/stipulation.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"

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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type stoponshortsolutions_solve(slice_index si, stip_length_type n)
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
    result = solve(slices[si].next1,n);
    if (result<=n && n<slices[si].u.branch.length)
      short_solution_found();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
