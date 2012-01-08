#include "stipulation/end_of_branch_goal.h"
#include "stipulation/branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
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

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
end_of_branch_goal_can_attack(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (max_unsolvable[nbply]<slack_length_battle
      || n<=max_unsolvable[nbply]) /* exact refutation */
  {
    switch (slice_has_solution(fork))
    {
      case has_solution:
        result = slack_length_battle;
        break;

      case has_no_solution:
        result = can_attack(next,n);
        break;

      case opponent_self_check:
        result = slack_length_battle-2;
        break;

      default:
        assert(0);
        result = slack_length_battle-2;
        break;
    }
  }
  else
    result = can_attack(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
end_of_branch_goal_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (max_unsolvable[nbply]<slack_length_battle
      || n<=max_unsolvable[nbply]) /* exact refutation */
  {
    switch (slice_solve(fork))
    {
      case has_solution:
        result = slack_length_battle;
        break;

      case has_no_solution:
        result = attack(next,n);
        break;

      case opponent_self_check:
        result = slack_length_battle-2;
        break;

      default:
        assert(0);
        result = slack_length_battle-2;
        break;
    }
  }
  else
    result = attack(next,n);

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
  slice_index const next = slices[si].u.fork.next;
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
  slice_index const next = slices[si].u.fork.next;

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
