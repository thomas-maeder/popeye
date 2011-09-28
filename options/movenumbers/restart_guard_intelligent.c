#include "options/movenumbers/restart_guard_intelligent.h"
#include "pydata.h"
#include "pymovenb.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STRestartGuardIntelligent slice.
 * @return allocated slice
 */
slice_index alloc_restart_guard_intelligent(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRestartGuardIntelligent);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean too_short(stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (OptFlag[restart])
  {
    stip_length_type min_length = 2*get_restart_number();
    if ((n-slack_length_help)%2==1)
      --min_length;
    result = n-slack_length_help<min_length;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type restart_guard_intelligent_help(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (too_short(n))
    result = n+2;
  else
    result = help(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
