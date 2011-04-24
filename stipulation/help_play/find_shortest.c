#include "stipulation/help_play/find_shortest.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/play.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/root.h"
#include "stipulation/fork_on_remaining.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STHelpFindShortest slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_help_find_shortest_slice(stip_length_type length,
                                           stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STHelpFindShortest,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void help_find_shortest_make_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  slice_index const next = slices[si].u.pipe.next;
  slice_index root;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  root = alloc_help_root_slice(length,min_length);

  if (length<slack_length_help+2)
    pipe_set_successor(root,next);
  else
  {
    slice_index const end = branch_find_slice(STEndOfAdapter,si);
    assert(end!=no_slice);
    stip_traverse_structure_children(si,st);
    assert(*root_slice!=no_slice);
    pipe_link(root,*root_slice);

    {
      slice_index const shortcut_proto = alloc_fork_on_remaining_slice(end,length-slack_length_help-1);
      help_branch_insert_slices(root,&shortcut_proto,1);
    }

    help_branch_shorten_slice(si);
  }

  *root_slice = root;

  pipe_unlink(slices[si].prev);
  dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type help_find_shortest_help(slice_index si, stip_length_type n)
{
  stip_length_type result = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result += (n-result)%2;

  while (result<=n)
    if (help(slices[si].u.pipe.next,result)==result)
      break;
    else
      result += 2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type help_find_shortest_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result += (n-result)%2;

  while (result<=n)
    if (can_help(slices[si].u.pipe.next,result)==result)
      break;
    else
      result += 2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
