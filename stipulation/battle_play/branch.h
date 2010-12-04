#if !defined(STIPULATION_BATTLE_PLAY_BRANCH_H)
#define STIPULATION_BATTLE_PLAY_BRANCH_H

#include "pystip.h"

/* This module provides functionality dealing with battle play
 * branches
 */

/* Allocate a branch consisting mainly of an attack move
 * @param  length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_attack_branch(stip_length_type length,
                                stip_length_type min_length);

/* Allocate a branch consisting mainly of an defense move
 * @param  length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies slice where the defense branch leads to
 * @return index of entry slice to allocated branch
 */
slice_index alloc_defense_branch(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index next);

/* Allocate a branch that represents battle play
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice of allocated branch
 */
slice_index alloc_battle_branch(stip_length_type length,
                                stip_length_type min_length);

/* Shorten a battle slice by 2 half moves
 * @param si identifies slice to be shortened
 */
void battle_branch_shorten_slice(slice_index si);

/* Determine the position where to insert a slice into an defense branch.
 * @param si entry slice of defense branch
 * @param type type of slice to be inserted
 * @return identifier of slice before which to insert; no_slice if no
 *         suitable position could be found
 */
slice_index find_defense_slice_insertion_pos(slice_index si, SliceType type);

/* Determine the position where to insert a slice into an attack branch.
 * @param si entry slice of attack branch
 * @param type type of slice to be inserted
 * @return identifier of slice before which to insert; no_slice if no
 *         suitable position could be found
 */
slice_index find_attack_slice_insertion_pos(slice_index si, SliceType type);

#endif
