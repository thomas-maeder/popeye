#include "stipulation/series_play/ready_for_series_move.h"
#include "trace.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/series_play/branch.h"

#include <assert.h>

/* Allocate a STReadyForSeriesMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_ready_for_series_move_slice(stip_length_type length,
                                              stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STReadyForSeriesMove,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void ready_for_series_move_make_root(slice_index si,
                                     stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;
  slice_index copy;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  copy = copy_slice(si);
  pipe_link(copy,*root_slice);
  *root_slice = copy;
  shorten_series_pipe(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Is it a dummy move that we are ready for?
 * @param si identifies slice
 */
boolean ready_for_series_move_is_move_dummy(slice_index si)
{
  boolean result = false;
  slice_index current = si;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  do
  {
    assert(slices[current].type!=STLeaf);
    if (slices[current].type==STSeriesDummyMove)
    {
      result = true;
      break;
    }
    else if (slices[current].type==STSeriesMove)
      break;
    else
      current = slices[current].u.pipe.next;
  } while (current!=si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
