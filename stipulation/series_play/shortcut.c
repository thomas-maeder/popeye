#include "stipulation/series_play/shortcut.h"
#include "stipulation/series_play/play.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STSeriesShortcut slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param short_sols identifies slice to delegate to when looking for
 *                   short solutions
 * @return allocated slice
 */
slice_index alloc_series_shortcut(stip_length_type length,
                                  slice_index short_sols)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",short_sols);
  TraceFunctionParamListEnd();

  result = alloc_branch(STSeriesShortcut,length,0);
  slices[result].u.shortcut.short_sols = short_sols;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_series_shortcut(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);
  stip_traverse_structure(slices[si].u.shortcut.short_sols,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves beyond a series shortcut slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_series_shortcut(slice_index si,
                                         stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==st->full_length)
    stip_traverse_moves_pipe(si,st);
  else
    stip_traverse_moves(slices[si].u.shortcut.short_sols,st);

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
stip_length_type series_shortcut_series(slice_index si, stip_length_type n)
{
  stip_length_type result;
  stip_length_type const full_length = slices[si].u.shortcut.length;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const short_sols = slices[si].u.shortcut.short_sols;
  slice_index next_branch = n<full_length ? short_sols : next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = series(next_branch,n);

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
stip_length_type series_shortcut_has_series(slice_index si, stip_length_type n)
{
  stip_length_type result;
  stip_length_type const full_length = slices[si].u.shortcut.length;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const short_sols = slices[si].u.shortcut.short_sols;
  slice_index next_branch = n<full_length ? short_sols : next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = has_series(next_branch,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
