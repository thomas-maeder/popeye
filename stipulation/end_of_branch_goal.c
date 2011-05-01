#include "stipulation/end_of_branch_goal.h"
#include "stipulation/branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/reflex_attack_solver.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/min_length_guard.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/series_play/branch.h"
#include "pypipe.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STEndOfBranchGoal slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_goal(slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STEndOfBranchGoal,proxy_to_avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STEndOfBranchForced slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_forced(slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STEndOfBranchForced,proxy_to_avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void end_of_branch_goal_make_root(slice_index si,
                                  stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index * const root_slice = st->param;
    slice_index const fork = slices[si].u.fork.fork;
    slice_index const fork_copy = stip_deep_copy(fork);
    slice_index const defense_root = alloc_end_of_branch_goal(fork_copy);
    link_to_branch(defense_root,*root_slice);
    *root_slice = defense_root;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
end_of_branch_goal_can_attack(slice_index si,
                              stip_length_type n,
                              stip_length_type n_max_unsolvable)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const fork = slices[si].u.fork.fork;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle
      || n<=n_max_unsolvable) /* exact refutation */
  {
    result = can_attack(fork,n,n_max_unsolvable);
    if (result>n)
      /* delegate to next even if (n==slack_length_battle) - we need
       * to distinguish between self-check and other ways of not
       * reaching the goal */
      result = can_attack(next,n,n_max_unsolvable);
  }
  else
    result = can_attack(next,n,n_max_unsolvable);

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
stip_length_type
end_of_branch_goal_attack(slice_index si,
                          stip_length_type n,
                          stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle
      || n<=n_max_unsolvable) /* exact refutation */
  {
    result = attack(fork,n,n_max_unsolvable);
    if (result>n)
      /* delegate to next even if (n==slack_length_battle) - we need
       * to distinguish between self-check and other ways of not
       * reaching the goal */
      result = attack(next,n,n_max_unsolvable);
  }
  else
    result = attack(next,n,n_max_unsolvable);

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
stip_length_type
end_of_branch_goal_defend(slice_index si,
                          stip_length_type n,
                          stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle
      && slice_solve(fork)==has_solution)
    result = slack_length_battle;
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
stip_length_type
end_of_branch_goal_can_defend(slice_index si,
                              stip_length_type n,
                              stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle
      && slice_has_solution(fork)==has_solution)
    result = slack_length_battle;
  else
    result = can_defend(next,n,n_max_unsolvable);

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
stip_length_type end_of_branch_goal_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const fork = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;
  has_solution_type avoided_sol;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  avoided_sol = (n<slack_length_help+2
                 ? slice_solve(fork)
                 : slice_has_solution(fork));
  switch (avoided_sol)
  {
    case opponent_self_check:
      result = n+4;
      break;

    case has_no_solution:
      result = help(next,n);
      break;

    case has_solution:
      result = n;
      break;

    default:
      assert(0);
      result = n+4;
      break;
  }

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
stip_length_type end_of_branch_goal_can_help(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
  slice_index const fork = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  switch (slice_has_solution(fork))
  {
    case opponent_self_check:
      result = n+4;
      break;

    case has_no_solution:
      result = can_help(next,n);
      break;

    case has_solution:
      result = n;
      break;

    default:
      assert(0);
      result = n+4;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type end_of_branch_goal_series(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const fork = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;
  has_solution_type avoided_sol;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  avoided_sol = (n==slack_length_series
                 ? slice_solve(fork)
                 : slice_has_solution(fork));
  switch (avoided_sol)
  {
    case opponent_self_check:
      result = n+2;
      break;

    case has_no_solution:
      result = series(next,n);
      break;

    case has_solution:
      result = n;
      break;

    default:
      assert(0);
      result = n+2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type end_of_branch_goal_has_series(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  slice_index const fork = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  switch (slice_has_solution(fork))
  {
    case opponent_self_check:
      result = n+2;
      break;

    case has_no_solution:
      result = has_series(next,n);
      break;

    case has_solution:
      result = n;
      break;

    default:
      assert(0);
      result = n+2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
