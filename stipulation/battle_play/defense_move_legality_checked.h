#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_LEGALITY_CHECKED_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_LEGALITY_CHECKED_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with STDefenseMoveLegalityChecked
 * stipulation slices.
 */

/* Allocate a STDefenseMoveLegalityChecked defender slice.
 * @return index of allocated slice
 */
slice_index alloc_defense_move_legality_checked_slice(void);

#endif
