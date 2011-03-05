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

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void ready_for_defense_make_setplay_slice(slice_index si,
                                          stip_structure_traversal *st);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void ready_for_defense_reduce_to_postkey_play(slice_index si,
                                              stip_structure_traversal *st);

#endif
