#include "solving/recursion_stopper.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STRecursionStopper slice
 * @return newly allocated slice
 */
slice_index alloc_recursion_stopper_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRecursionStopper);

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
stip_length_type recursion_stopper_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

#if defined(DOTRACE)
  /* empirically determined one 1 workstation */
  ply const stop_at_ply = 250;
#else
  ply const stop_at_ply = maxply;
#endif

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (nbply>stop_at_ply)
    FtlMsg(ChecklessUndecidable);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
