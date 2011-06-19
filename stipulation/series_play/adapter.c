#include "stipulation/series_play/adapter.h"
#include "stipulation/branch.h"
#include "stipulation/series_play/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STSeriesAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_series_adapter_slice(stip_length_type length,
                                       stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STSeriesAdapter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Attempt to add set play to an attack stipulation (battle play, not
 * postkey only)
 * @param adapter identifies the root from which to apply set play
 * @param st address of structure representing traversal
 */
void series_adapter_apply_setplay(slice_index adapter, stip_structure_traversal *st)
{
  slice_index * const setplay_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  *setplay_slice = series_make_setplay(slices[adapter].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
