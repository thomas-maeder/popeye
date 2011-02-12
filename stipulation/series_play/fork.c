#include "stipulation/series_play/fork.h"
#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/series_play/branch.h"
#include "stipulation/series_play/play.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STSeriesFork slice.
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_series_fork_slice(slice_index to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STSeriesFork,0,0,to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void series_fork_make_setplay(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (get_max_nr_moves(slices[si].u.branch_fork.towards_goal)>0)
  {
    slice_index * const setplay_slice = st->param;

    /* build a 0 move series branch that leads to the successive branch */
    slice_index const branch = alloc_series_branch(slack_length_series,
                                                   slack_length_series);
    series_branch_set_goal_slice(branch,
                                 slices[si].u.branch_fork.towards_goal);

    *setplay_slice = alloc_proxy_slice();
    pipe_link(*setplay_slice,branch);
    stip_insert_root_slices(*setplay_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_series_fork(slice_index branch,
                                         stip_structure_traversal *st)
{
  stip_traverse_structure_pipe(branch,st);
  stip_traverse_structure(slices[branch].u.branch_fork.towards_goal,st);
}

/* Traversal of the moves beyond a series fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_series_fork(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_init_full_length(si,st);

  if (st->remaining<slack_length_series+2)
    stip_traverse_moves_branch(slices[si].u.branch_fork.towards_goal,st);
  else
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
stip_length_type series_fork_solve_in_n(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  switch (n)
  {
    case slack_length_series:
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
      break;

    case slack_length_series+1:
      result = series_solve_in_n(to_goal,n);
      break;

    default:
      result = series_solve_in_n(next,n);
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
stip_length_type series_fork_has_solution_in_n(slice_index si,
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

  switch (n)
  {
    case slack_length_series:
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
      break;

    case slack_length_series+1:
      result = series_has_solution_in_n(to_goal,n);
      break;

    default:
      result = series_has_solution_in_n(next,n);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
