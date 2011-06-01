#if !defined(STIPULATION_SERIES_PLAY_ADAPTER_H)
#define STIPULATION_SERIES_PLAY_ADAPTER_H

#include "pyslice.h"
#include "stipulation/series_play/play.h"

/* This module provides functionality dealing with STSeriesAdapter
 * stipulation slices. STSeriesAdapter slices switch from general play to series
 * play.
 */

/* Allocate a STSeriesAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_series_adapter_slice(stip_length_type length,
                                       stip_length_type min_length);

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 * @param st address of structure holding the traversal state
 */
void series_adapter_make_root(slice_index si, stip_structure_traversal *st);

/* Wrap the slices representing the nested slices
 * @param adapter identifies attack adapter slice
 * @param st address of structure holding the traversal state
 */
void series_adapter_make_intro(slice_index adapter,
                               stip_structure_traversal *st);

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_series_adpater(slice_index si,
                                            stip_structure_traversal *st);

/* Traversal of the moves of some adapter slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_series_adapter(slice_index si,
                                        stip_moves_traversal *st);

/* Attempt to add set play to an attack stipulation (battle play, not
 * postkey only)
 * @param si identifies the root from which to apply set play
 * @param st address of structure representing traversal
 */
void series_adapter_apply_setplay(slice_index si, stip_structure_traversal *st);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type series_adapter_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type series_adapter_has_solution(slice_index si);

#endif
