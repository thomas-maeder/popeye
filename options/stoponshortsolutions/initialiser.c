#include "options/stoponshortsolutions/initialiser.h"
#include "stipulation/stipulation.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

/* Allocate a STStopOnShortSolutionsInitialiser slice.
 * @return allocated slice
 */
slice_index alloc_stoponshortsolutions_initialiser_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STStopOnShortSolutionsInitialiser);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type stoponshortsolutions_initialiser_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  reset_short_solution_found_in_phase();

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
