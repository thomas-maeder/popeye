#include "solving/fork_on_remaining.h"
#include "pypipe.h"
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

  result = alloc_slice(STForkOnRemaining);
  slices[result].u.fork_on_remaining.op1 = op1;
  slices[result].u.fork_on_remaining.op2 = op2;
  slices[result].u.fork_on_remaining.threshold = threshold;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_fork_on_remaining(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.fork_on_remaining.op1,st);
  stip_traverse_structure(slices[si].u.fork_on_remaining.op2,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
fork_on_remaining_can_attack(slice_index si,
                             stip_length_type n,
                             stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const op1 = slices[si].u.fork_on_remaining.op1;
  slice_index const op2 = slices[si].u.fork_on_remaining.op2;
  stip_length_type const threshold = slices[si].u.fork_on_remaining.threshold;
  slice_index const succ = n<=slack_length_battle+threshold ? op2 : op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>slack_length_battle);
  result = can_attack(succ,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type fork_on_remaining_attack(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const op1 = slices[si].u.fork_on_remaining.op1;
  slice_index const op2 = slices[si].u.fork_on_remaining.op2;
  stip_length_type const threshold = slices[si].u.fork_on_remaining.threshold;
  slice_index const succ = n<=slack_length_battle+threshold ? op2 : op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>slack_length_battle);
  result = attack(succ,n,n_max_unsolvable);

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
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type fork_on_remaining_defend(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const op1 = slices[si].u.fork_on_remaining.op1;
  slice_index const op2 = slices[si].u.fork_on_remaining.op2;
  stip_length_type const threshold = slices[si].u.fork_on_remaining.threshold;
  slice_index const succ = n<=slack_length_battle+threshold ? op2 : op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>slack_length_battle);
  result = defend(succ,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type fork_on_remaining_can_defend(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const op1 = slices[si].u.fork_on_remaining.op1;
  slice_index const op2 = slices[si].u.fork_on_remaining.op2;
  stip_length_type const threshold = slices[si].u.fork_on_remaining.threshold;
  slice_index const succ = n<=slack_length_battle+threshold ? op2 : op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>slack_length_battle);
  result = can_defend(succ,n,n_max_unsolvable);

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
stip_length_type fork_on_remaining_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const op1 = slices[si].u.fork_on_remaining.op1;
  slice_index const op2 = slices[si].u.fork_on_remaining.op2;
  stip_length_type const threshold = slices[si].u.fork_on_remaining.threshold;
  slice_index const succ = n<=slack_length_help+threshold ? op2 : op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);
  result = help(succ,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type fork_on_remaining_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const op1 = slices[si].u.fork_on_remaining.op1;
  slice_index const op2 = slices[si].u.fork_on_remaining.op2;
  stip_length_type const threshold = slices[si].u.fork_on_remaining.threshold;
  slice_index const succ = n<=slack_length_help+threshold ? op2 : op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);
  result = can_help(succ,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
