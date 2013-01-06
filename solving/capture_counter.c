#include "solving/capture_counter.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>

/* current value of the count */
unsigned int capture_counter_count;

/* stop the move iteration once capture_counter_count exceeds this number */
unsigned int capture_counter_interesting;

/* Allocate a STCaptureCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_capture_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCaptureCounter);

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
stip_length_type capture_counter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (just_moved==Black ? pprise[nbply]>=roib : pprise[nbply]<=roib)
    ++capture_counter_count;

  TraceValue("%u",capture_counter_count);
  TraceValue("%u\n",capture_counter_interesting);
  if (capture_counter_count<=capture_counter_interesting)
    result = solve(slices[si].next1,n);
  else
    /* stop the iteration */
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
