#if !defined(STIPULATION_HELP_PLAY_READY_FOR_HELP_MOVE_H)
#define STIPULATION_HELP_PLAY_READY_FOR_HELP_MOVE_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STReadyForHelpMove
 * stipulation slices.
 */

/* Allocate a STReadyForHelpMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_ready_for_help_move_slice(stip_length_type length,
                                            stip_length_type min_length);

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void ready_for_help_move_make_root(slice_index si,
                                   stip_structure_traversal *st);

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void ready_for_help_move_make_setplay_slice(slice_index si,
                                            stip_structure_traversal *st);

#endif
