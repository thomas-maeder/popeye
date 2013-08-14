#include "stipulation/boolean/not.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "solving/solve.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a not slice.
 * @return index of allocated slice
 */
slice_index alloc_not_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STNot);

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
stip_length_type not_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  stip_length_type next_result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  next_result = solve(slices[si].next1,n);
  if (next_result>n)
    result = n;
  else if (next_result>=slack_length)
    result = n+2;
  else
    result = next_result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
