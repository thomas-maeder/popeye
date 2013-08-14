#include "output/plaintext/tree/try_writer.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/battle_play/try.h"
#include "solving/trivial_end_filter.h"
#include "output/plaintext/tree/key_writer.h"
#include "debugging/trace.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"

#include <assert.h>

/* Allocate a STTryWriter defender slice.
 * @return index of allocated slice
 */
slice_index alloc_try_writer(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STTryWriter);

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
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type try_writer_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (table_length(refutations)>0)
  {
    StdString(" ?");
    result = solve(slices[si].next1,n);
  }
  else
	  result = key_writer_solve(si,n);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}
