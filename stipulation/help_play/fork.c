#include "stipulation/help_play/fork.h"
#include "pybrafrk.h"
#include "stipulation/help_play/play.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>

/* **************** Initialisation ***************
 */

/* Allocate a STHelpFork slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_help_fork_slice(stip_length_type length,
                                  stip_length_type min_length,
                                  slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  assert(slices[proxy_to_goal].type==STProxy);

  if (min_length+1<slack_length_help)
    min_length += 2;
  result = alloc_branch_fork(STHelpFork,length,min_length,proxy_to_goal);
  
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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index * const root = st->param;
    slice_index const shortcut = branch_find_slice(STHelpShortcut,*root);
    assert(shortcut!=no_slice);
    slices[shortcut].u.shortcut.short_sols = si;
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
void stip_traverse_moves_help_fork(slice_index si, stip_move_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==0)
  {
    st->full_length = slices[si].u.branch.length;
    TraceValue("->%u\n",st->full_length);
    st->remaining = slices[si].u.branch.length;
  }

  if (st->remaining==slack_length_help)
  {
    stip_length_type const save_remaining = st->remaining;
    stip_length_type const save_full_length = st->full_length;
    ++st->level;
    st->remaining = 0;
    stip_traverse_moves(slices[si].u.branch_fork.towards_goal,st);
    st->full_length = save_full_length;
    st->remaining = save_remaining;
    --st->level;
  }
  else
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

  if (n==slack_length_help)
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

  if (n==slack_length_help)
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
