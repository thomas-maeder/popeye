#include "optimisations/goals/castling/help_filter.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/help_play/play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STCastlingHelpFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_castling_help_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCastlingHelpFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type castling_help_filter_solve_in_n(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_help+1);

  if ((slices[si].starter==White
       ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)>ke1_cancastle
       : TSTFLAGMASK(castling_flag[nbply],bl_castlings)>ke8_cancastle))
    result = help_solve_in_n(slices[si].u.pipe.next,slack_length_help+1);
  else
    result = slack_length_help+3;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type castling_help_filter_has_solution_in_n(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_help+1);

  if ((slices[si].starter==White
       ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)>ke1_cancastle
       : TSTFLAGMASK(castling_flag[nbply],bl_castlings)>ke8_cancastle))
    result = help_has_solution_in_n(slices[si].u.pipe.next,
                                    slack_length_help+1);
  else
    result = slack_length_help+3;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
