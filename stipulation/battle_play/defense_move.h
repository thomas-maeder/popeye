#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_H

#include "boolean.h"
#include "pydirect.h"

/* This module provides functionality dealing with the defending side
 * in STAttackMove stipulation slices.
 */

/* Allocate a STDefenseMove defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_move_slice(stip_length_type length,
                                     stip_length_type min_length);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 * @return true iff slice has been successfully traversed
 */
boolean defense_move_insert_root(slice_index si, slice_traversal *st);

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean defense_move_defend_in_n(slice_index si, stip_length_type n);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int defense_move_can_defend_in_n(slice_index si,
                                          stip_length_type n,
                                          unsigned int max_result);

#endif
