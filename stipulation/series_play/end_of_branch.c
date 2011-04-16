#include "stipulation/series_play/end_of_branch.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/series_play/adapter.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STEndOfSeriesBranch slice.
 * @param proxy_to_next identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_end_of_series_branch_slice(slice_index proxy_to_next)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_next);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STEndOfSeriesBranch,proxy_to_next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void end_of_series_branch_make_setplay(slice_index si,
                                       stip_structure_traversal *st)
{
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
  slice_index * const setplay_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *setplay_slice = alloc_series_adapter_slice(slack_length_series,
                                              slack_length_series);
  pipe_link(*setplay_slice,alloc_end_of_series_branch_slice(to_goal));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves beyond a series fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_end_of_series_branch(slice_index si,
                                              stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining<=1)
    stip_traverse_moves_branch(slices[si].u.branch_fork.towards_goal,st);

  if (st->remaining>0)
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type end_of_series_branch_series_in_n(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slack_length_series)
    switch (slice_solve(to_goal))
    {
      case has_solution:
        result = n;
        break;

      case has_no_solution:
        result = n+1;
        break;

      case opponent_self_check:
        result = n+2;
        break;

      default:
        assert(0);
        result = n+2;
        break;
    }
  else
    result = series_solve_in_n(next,n);

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
stip_length_type end_of_series_branch_has_series_in_n(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slack_length_series)
    switch (slice_has_solution(to_goal))
    {
      case has_solution:
        result = n;
        break;

      case has_no_solution:
        result = n+1;
        break;

      case opponent_self_check:
        result = n+2;
        break;

      default:
        assert(0);
        result = n+2;
        break;
    }
  else
    result = series_has_solution_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
