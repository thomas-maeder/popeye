#if !defined(STIPULATION_SERIES_PLAY_MOVE_H)
#define STIPULATION_SERIES_PLAY_MOVE_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STSeriesMove
 * stipulation slices.
 */

/* Allocate a STSeriesMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_series_move_slice(stip_length_type length,
                                    stip_length_type min_length);

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void series_move_make_root(slice_index si, stip_structure_traversal *st);

/* Spin off set play
 * @param si slice index
 * @param st state of traversal
 */
void series_move_apply_setplay(slice_index si, stip_structure_traversal *st);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void series_move_detect_starter(slice_index si, stip_structure_traversal *st);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_move_series(slice_index si, stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_move_has_series(slice_index si, stip_length_type n);

#endif
