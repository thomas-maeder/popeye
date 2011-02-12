#include "stipulation/series_play/find_shortest.h"
#include "stipulation/branch.h"
#include "stipulation/series_play/play.h"
#include "stipulation/series_play/branch.h"
#include "stipulation/series_play/root.h"
#include "stipulation/series_play/shortcut.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STSeriesFindShortest slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_series_find_shortest_slice(stip_length_type length,
                                             stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STSeriesFindShortest,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void series_find_shortest_make_root(slice_index si, stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  slice_index ready;
  slice_index root;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ready = branch_find_slice(STReadyForSeriesMove,si);
  assert(ready!=no_slice);

  root = alloc_series_root_slice(length,min_length);

  if (length<slack_length_series+2)
    pipe_set_successor(root,slices[ready].u.pipe.next);
  else
  {
    slice_index const shortcut = alloc_series_shortcut(length,min_length,ready);
    pipe_link(root,shortcut);
    stip_traverse_structure_children(si,st);
    assert(state->result!=no_slice);
    pipe_link(shortcut,state->result);
    shorten_series_pipe(si);
  }

  state->result = root;

  dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the solution(s) in a series stipulation
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_find_shortest_solve_in_n(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  while (result<=n)
    if (series_solve_in_n(slices[si].u.pipe.next,result)==result)
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
stip_length_type series_find_shortest_has_solution_in_n(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  while (result<=n)
    if (series_has_solution_in_n(slices[si].u.pipe.next,result)==result)
      break;
    else
      result += 2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
