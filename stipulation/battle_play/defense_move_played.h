#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_PLAYED_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_PLAYED_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with STDefenseMovePlayed
 * stipulation slices.
 */

/* Allocate a STDefenseMovePlayed defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_move_played_slice(stip_length_type length,
                                            stip_length_type min_length);

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void defense_move_played_make_setplay_slice(slice_index si,
                                            stip_structure_traversal *st);

#endif
