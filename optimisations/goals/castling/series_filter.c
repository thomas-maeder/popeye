#include "optimisations/goals/castling/series_filter.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/series_play/play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STCastlingSeriesFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_castling_series_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCastlingSeriesFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a series stipulation
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type castling_series_filter_solve_in_n(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_series+1);

  if ((slices[si].starter==White
       ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)>ke1_cancastle
       : TSTFLAGMASK(castling_flag[nbply],bl_castlings)>ke8_cancastle))
    result = series_solve_in_n(slices[si].u.pipe.next,n);
  else
    result = slack_length_series+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type castling_series_filter_has_solution_in_n(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if ((slices[si].starter==White
       ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)>ke1_cancastle
       : TSTFLAGMASK(castling_flag[nbply],bl_castlings)>ke8_cancastle))
    result = series_has_solution_in_n(slices[si].u.pipe.next,n);
  else
    result = slack_length_series+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
