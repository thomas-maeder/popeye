#if !defined(STIPULATION_BATTLE_PLAY_BRANCH_H)
#define STIPULATION_BATTLE_PLAY_BRANCH_H

#include "pystip.h"

/* This module provides functionality dealing with battle play
 * branches
 */

/* Allocate a branch consisting mainly of an defense move
 * @param next identifies the slice that the defense branch lead sto
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_defense_branch(slice_index next,
                                 stip_length_type length,
                                 stip_length_type min_length);

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

/* Insert slices into a battle branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by battle_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void battle_branch_insert_slices(slice_index si,
                                 slice_index const prototypes[],
                                 unsigned int nr_prototypes);

/* Insert slices into a battle branch; the elements of
 * prototypes are *not* deallocated by battle_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void battle_branch_insert_slices_nested(slice_index si,
                                        slice_index const prototypes[],
                                        unsigned int nr_prototypes);

#endif
