#include "stipulation/battle_play/root_attack_fork.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STRootAttackFork slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_next identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_root_attack_fork_slice(stip_length_type length,
                                         stip_length_type min_length,
                                         slice_index proxy_to_next)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_next);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STRootAttackFork,length,min_length,proxy_to_next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void root_attack_fork_reduce_to_postkey_play(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);
  dealloc_slices(slices[si].u.branch_fork.towards_goal);
  dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves beyond a series fork slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_root_attack_fork(slice_index si,
                                          stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_init_full_length(si,st);
  stip_traverse_moves_branch(slices[si].u.branch_fork.towards_goal,st);
  stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
root_attack_fork_solve_in_n(slice_index si,
                            stip_length_type n,
                            stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.branch_fork.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>slack_length_battle);

  if (n_max_unsolvable==slack_length_battle)
  {
    result = attack_solve_in_n(to_goal,
                               slack_length_battle+1,slack_length_battle);

    {
      stip_length_type const
          result_next = attack_solve_in_n(next,n,slack_length_battle+1);
      if (result_next<result)
        result = result_next;
    }
  }
  else
    result = attack_solve_in_n(slices[si].u.branch_fork.next,
                               n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
