#include "stipulation/constraint.h"
#include "stipulation/branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch_goal.h"
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


/* Allocate a STConstraint slice
 * @param proxy_to_condition prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_constraint_slice(slice_index proxy_to_condition)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_condition);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STConstraint,proxy_to_condition);

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
void constraint_apply_postkeyplay(slice_index si, stip_structure_traversal *st)
{
  slice_index * const postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  dealloc_slices(slices[si].u.fork.fork);
  trash_for_postkey_play(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type constraint_solve(slice_index si)
{
  has_solution_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slice_solve(condition))
  {
    case has_no_solution:
      result = slice_solve(next);
      break;

    case has_solution:
      result = has_no_solution;
      break;

    case opponent_self_check:
      result = opponent_self_check;
      break;

    default:
      assert(0);
      result = opponent_self_check;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type constraint_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(condition))
  {
    case has_no_solution:
      result = slice_has_solution(next);
      break;

    case has_solution:
      result = has_no_solution;
      break;

    case opponent_self_check:
      result = opponent_self_check;
      break;

    default:
      assert(0);
      result = opponent_self_check;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
stip_length_type constraint_can_attack(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = slack_length_battle-2;
      break;

    case has_solution:
      result = n+2;
      break;

    case has_no_solution:
      result = can_attack(next,n,n_max_unsolvable);
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
stip_length_type constraint_attack(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  switch (n==n_max_unsolvable
          ? slice_solve(condition) /* we are solving refutations */
          : slice_has_solution(condition))
  {
    case opponent_self_check:
      result = slack_length_battle-2;
      break;

    case has_no_solution:
      result = attack(next,n,n_max_unsolvable);
      break;

    case has_solution:
      result = n+2;
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

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type constraint_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  /* TODO exact - but what does it mean??? */
  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = n+4;
      break;

    case has_solution:
      result = n+2;
      break;

    case has_no_solution:
      result = help(next,n);
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
stip_length_type constraint_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  /* TODO exact - but what does it mean??? */
  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = n+4;
      break;

    case has_solution:
      result = n+2;
      break;

    case has_no_solution:
      result = can_help(next,n);
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
stip_length_type constraint_series(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = n+2;
      break;

    case has_solution:
      result = n+1;
      break;

    case has_no_solution:
      result = series(next,n);
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
stip_length_type constraint_has_series(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = n+2;
      break;

    case has_solution:
      result = n+1;
      break;

    case has_no_solution:
      result = has_series(next,n);
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
