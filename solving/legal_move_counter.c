#include "solving/legal_move_counter.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* current value of the count */
unsigned int legal_move_counter_count;

/* stop the move iteration once legal_move_counter_count exceeds this number */
unsigned int legal_move_counter_interesting;

/* Allocate a STLegalMoveCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_legal_move_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STLegalMoveCounter);

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
stip_length_type legal_move_counter_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  ++legal_move_counter_count;

  if (legal_move_counter_count<=legal_move_counter_interesting)
    /* keep the iteration going */
    result = n+1;
  else
    result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
