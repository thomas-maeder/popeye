#include "optimisations/optimisation_fork.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STOptimisationFork slice.
 * @param optimisation identifies slice leading towards goal
 * @param threshold at which move should we optimise
 * @return index of allocated slice
 */
slice_index alloc_optimisation_fork_slice(slice_index optimisation,
                                          stip_length_type threshold)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",optimisation);
  TraceFunctionParam("%u",threshold);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOptimisationFork);
  slices[result].u.optimisation_fork.optimisation = optimisation;
  slices[result].u.optimisation_fork.threshold = threshold;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traversal of the moves beyond an optimisation fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_optimisation_fork(slice_index si,
                                           stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining<=slices[si].u.optimisation_fork.threshold)
    stip_traverse_moves(slices[si].u.optimisation_fork.optimisation,st);
  else
    stip_traverse_moves_pipe(si,st);

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
optimisation_fork_can_attack(slice_index si,
                             stip_length_type n,
                             stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>slack_length_battle);

  if (n<=slack_length_battle+slices[si].u.optimisation_fork.threshold)
    result = can_attack(slices[si].u.optimisation_fork.optimisation,
                        slack_length_battle+1,
                        n_max_unsolvable);
  else
    result = can_attack(slices[si].u.optimisation_fork.next,n,n_max_unsolvable);

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
stip_length_type optimisation_fork_attack(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>slack_length_battle);

  if (n<=slack_length_battle+slices[si].u.optimisation_fork.threshold)
    result = attack(slices[si].u.optimisation_fork.optimisation,
                    slack_length_battle+1,
                    n_max_unsolvable);
  else
    result = attack(slices[si].u.optimisation_fork.next,n,n_max_unsolvable);

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
stip_length_type optimisation_fork_defend(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.optimisation_fork.next;
  slice_index const optimisation = slices[si].u.optimisation_fork.optimisation;
  stip_length_type const threshold = slices[si].u.optimisation_fork.threshold;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>slack_length_battle);

  if (n<=slack_length_battle+threshold)
    result = defend(optimisation,n,n_max_unsolvable);
  else
    result = defend(next,n,n_max_unsolvable);

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
stip_length_type optimisation_fork_can_defend(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.optimisation_fork.next;
  slice_index const optimisation = slices[si].u.optimisation_fork.optimisation;
  stip_length_type const threshold = slices[si].u.optimisation_fork.threshold;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>slack_length_battle);

  if (n<=slack_length_battle+threshold)
    result = can_defend(optimisation,n,n_max_unsolvable);
  else
    result = can_defend(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
