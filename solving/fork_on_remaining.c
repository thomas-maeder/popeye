#include "solving/fork_on_remaining.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/binary.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STForkOnRemaining slice.
 * @param op1 identifies direction taken if threshold is not met
 * @param op2 identifies direction taken if threshold is met
 * @param threshold at which move should we continue with op2?
 * @return index of allocated slice
 */
slice_index alloc_fork_on_remaining_slice(slice_index op1,
                                          slice_index op2,
                                          stip_length_type threshold)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParam("%u",threshold);
  TraceFunctionParamListEnd();

  result = alloc_binary_slice(STForkOnRemaining,op1,op2);
  slices[result].u.fork_on_remaining.threshold = threshold;

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
stip_length_type fork_on_remaining_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const op1 = slices[si].next1;
  slice_index const op2 = slices[si].next2;
  stip_length_type const threshold = slices[si].u.fork_on_remaining.threshold;
  slice_index const succ = n<=slack_length+threshold ? op2 : op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(succ,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
