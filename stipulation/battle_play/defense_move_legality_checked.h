#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_LEGALITY_CHECKED_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_LEGALITY_CHECKED_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with STDefenseMoveLegalityChecked
 * stipulation slices.
 */

/* Allocate a STDefenseMoveLegalityChecked defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index
alloc_defense_move_legality_checked_slice(stip_length_type length,
                                          stip_length_type min_length);

/* Create the root slices sequence for a battle play branch; shorten
 * the non-root slices by the moves represented by the root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void defense_move_legality_checked_make_root(slice_index si,
                                             stip_structure_traversal *st);

#endif
