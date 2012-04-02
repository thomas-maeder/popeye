#include "solving/fork_on_remaining.h"
#include "stipulation/boolean/binary.h"
#include "trace.h"

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

/* Traversal of the moves beyond an STForkOnRemaining slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_fork_on_remaining(slice_index si,
                                           stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining<=slices[si].u.fork_on_remaining.threshold)
    stip_traverse_moves(slices[si].u.fork_on_remaining.op2,st);
  else
    stip_traverse_moves(slices[si].u.fork_on_remaining.op1,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type fork_on_remaining_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const op1 = slices[si].u.fork_on_remaining.op1;
  slice_index const op2 = slices[si].u.fork_on_remaining.op2;
  stip_length_type const threshold = slices[si].u.fork_on_remaining.threshold;
  slice_index const succ = n<=slack_length+threshold ? op2 : op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(succ,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type fork_on_remaining_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const op1 = slices[si].u.fork_on_remaining.op1;
  slice_index const op2 = slices[si].u.fork_on_remaining.op2;
  stip_length_type const threshold = slices[si].u.fork_on_remaining.threshold;
  slice_index const succ = n<=slack_length+threshold ? op2 : op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length);
  result = defend(succ,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
