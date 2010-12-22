#include "stipulation/battle_play/attack_fork.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/root_attack_fork.h"
#include "stipulation/battle_play/attack_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STAttackFork slice.
 * @param proxy_to_next identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_attack_fork_slice(slice_index proxy_to_next)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_next);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STAttackFork,0,0,proxy_to_next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void attack_fork_make_root(slice_index si, stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
  slice_index attack_root;
  slice_index root_to_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(to_goal,st);
  root_to_goal = state->result;

  stip_traverse_structure_pipe(si,st);

  attack_root = alloc_root_attack_fork_slice(root_to_goal);
  pipe_link(attack_root,state->result);
  state->result = attack_root;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves beyond an attack fork
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_attack_fork(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining<=slack_length_battle+1)
    stip_traverse_moves_branch(slices[si].u.branch_fork.towards_goal,st);

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
attack_fork_has_solution_in_n(slice_index si,
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

  if (n_max_unsolvable==slack_length_battle)
  {
    result = attack_has_solution_in_n(slices[si].u.branch_fork.towards_goal,
                                      slack_length_battle+1,
                                      slack_length_battle);
    if (result>slack_length_battle+1)
      result = attack_has_solution_in_n(slices[si].u.branch_fork.next,
                                        n,
                                        slack_length_battle+1);
  }
  else
    result = attack_has_solution_in_n(slices[si].u.branch_fork.next,
                                      n,
                                      n_max_unsolvable);

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
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_fork_solve_in_n(slice_index si,
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

  if (n_max_unsolvable==slack_length_battle)
  {
    result = attack_solve_in_n(slices[si].u.branch_fork.towards_goal,
                               slack_length_battle+1,slack_length_battle);
    if (result>slack_length_battle+1)
      result = attack_solve_in_n(slices[si].u.branch_fork.next,
                                 n,slack_length_battle+1);
  }
  else
    result = attack_solve_in_n(slices[si].u.branch_fork.next,
                               n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
