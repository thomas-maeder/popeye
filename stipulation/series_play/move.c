#include "stipulation/series_play/move.h"
#include "stipulation/series_play/branch.h"
#include "pydata.h"
#include "pyproc.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STSeriesMove slice.
 * @return index of allocated slice
 */
slice_index alloc_series_move_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STSeriesMove);

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
stip_length_type series_move_series(slice_index si, stip_length_type n)
{
  stip_length_type result = n+1;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && series(next,n-1)==n-1)
      result = n;

    repcoup();
  }

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
stip_length_type series_move_has_series(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result = n+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  while (encore())
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && has_series(next,n-1)==n-1)
    {
      result = n;
      repcoup();
      break;
    }
    else
      repcoup();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
