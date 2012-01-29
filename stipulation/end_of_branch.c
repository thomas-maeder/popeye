#include "stipulation/end_of_branch.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "solving/fork_on_remaining.h"
#include "trace.h"

#include <assert.h>

/* **************** Initialisation ***************
 */

/* Allocate a STEndOfBranch slice.
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_slice(slice_index to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STEndOfBranch,to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_end_of_branch(slice_index si,
                                           stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);
  stip_traverse_structure_next_branch(si,st);
  if (slices[si].u.fork.tester!=no_slice)
    stip_traverse_structure_next_tester(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves beyond a help fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_end_of_branch(slice_index si,
                                       stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining<=1)
    stip_traverse_moves_branch(slices[si].u.fork.fork,st);

  stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void end_of_branch_fork_inserter_end_of_branch(slice_index si,
                                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_help)
  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,1);
    pipe_link(slices[si].prev,fork);
    pipe_append(si,proxy1);
    pipe_link(proxy2,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const end_of_branch_fork_inserters[] =
{
  { STEndOfBranch,             &end_of_branch_fork_inserter_end_of_branch },
  { STEndOfBranchGoalImmobile, &end_of_branch_fork_inserter_end_of_branch }
};

enum
{
  nr_end_of_branch_fork_inserters = (sizeof end_of_branch_fork_inserters
                                     / sizeof end_of_branch_fork_inserters[0])
};

/* Instrument STEndOfBranch (and STEndOfBranchGoalImmobile) slices with the
 * necessary STForkOnRemaining slices
 * @param root_slice identifes root slice of stipulation
 */
void stip_insert_end_of_branch_forks(slice_index root_slice)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    end_of_branch_fork_inserters,
                                    nr_end_of_branch_fork_inserters);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type end_of_branch_can_attack(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (max_unsolvable<slack_length_battle
      || n<=max_unsolvable) /* exact refutation */
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
stip_length_type end_of_branch_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (max_unsolvable<slack_length_battle
      || n<=max_unsolvable) /* exact refutation */
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

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type end_of_branch_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (max_unsolvable<slack_length_battle
      && slice_solve(fork)==has_solution)
    result = slack_length_battle;
  else
    result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type end_of_branch_can_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const tester = slices[si].u.fork.tester;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (max_unsolvable<slack_length_battle
      && slice_has_solution(tester)==has_solution)
    result = slack_length_battle;
  else
    result = can_defend(next,n);

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
stip_length_type end_of_branch_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);
  assert(n<slack_length_help+2);

  switch (slice_solve(fork))
  {
    case has_solution:
      result = n;
      break;

    case has_no_solution:
      result = help(next,n);
      break;

    case opponent_self_check:
      result = n+4;
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
stip_length_type end_of_branch_can_help(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);
  assert(n<slack_length_help+2);

  switch (slice_has_solution(fork))
  {
    case has_solution:
      result = n;
      break;

    case has_no_solution:
      result = can_help(next,n);
      break;

    case opponent_self_check:
      result = n+4;
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
