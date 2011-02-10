#if !defined(STIPULATION_BATTLE_PLAY_READY_FOR_DEFENSE_H)
#define STIPULATION_BATTLE_PLAY_READY_FOR_DEFENSE_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with the defending side
 * in STReadyForDefense stipulation slices.
 */

/* Allocate a STReadyForDefense defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_ready_for_defense_slice(stip_length_type length,
                                          stip_length_type min_length);

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void ready_for_defense_make_root(slice_index si, stip_structure_traversal *st);

/* Traversal of the moves beyond a slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_ready_for_defense(slice_index si,
                                           stip_moves_traversal *st);

#endif
