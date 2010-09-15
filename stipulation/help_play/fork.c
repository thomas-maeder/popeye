#include "stipulation/help_play/fork.h"
#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/play.h"
#include "stipulation/help_play/root.h"
#include "trace.h"

#include <assert.h>

/* **************** Initialisation ***************
 */

/* Allocate a STHelpFork slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_help_fork_slice(stip_length_type length,
                                  stip_length_type min_length,
                                  slice_index to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STHelpFork,length,min_length,to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void help_fork_make_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root = st->param;
  stip_length_type min_length = slices[si].u.branch.min_length;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length<slack_length_help+2)
  {
    *root = alloc_help_root_slice(length,min_length);
    pipe_set_successor(*root,si);
  }
  else
  {
    stip_traverse_structure_pipe(si,st);

    {
      slice_index const shortcut = branch_find_slice(STHelpShortcut,*root);
      assert(shortcut!=no_slice);
      slices[shortcut].u.shortcut.short_sols = si;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_help_fork(slice_index branch,
                                       stip_structure_traversal *st)
{
  stip_traverse_structure_pipe(branch,st);
  stip_traverse_structure(slices[branch].u.branch_fork.towards_goal,st);
}

/* Traversal of the moves beyond a help fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_help_fork(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_init_full_length(si,st);

  if (st->remaining<=slack_length_help+2)
    stip_traverse_moves_branch(slices[si].u.branch_fork.towards_goal,st);

  if (st->remaining>slack_length_help+1)
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type help_fork_solve_in_n(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (n<slack_length_help+3)
    switch (slice_solve(slices[si].u.branch_fork.towards_goal))
    {
      case has_solution:
        result = n;
        break;

      case has_no_solution:
        result = n+2;
        break;

      case opponent_self_check:
        result = n+4;
        break;

      default:
        assert(0);
        result = n+4;
        break;
    }
  else
    result = help_solve_in_n(slices[si].u.pipe.next,n);

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
stip_length_type help_fork_has_solution_in_n(slice_index si,
                                             stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (n<slack_length_help+3)
    switch (slice_has_solution(to_goal))
    {
      case has_solution:
        result = n;
        break;

      case has_no_solution:
        result = n+2;
        break;

      case opponent_self_check:
        result = n+4;
        break;

      default:
        assert(0);
        result = n+4;
        break;
    }
  else
    result = help_has_solution_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
