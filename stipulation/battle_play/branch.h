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

/* Determine the position where to insert a slice into an attack branch.
 * @param si entry slice of attack branch
 * @param type type of slice to be inserted
 * @return identifier of slice before which to insert; no_slice if no
 *         suitable position could be found
 */
slice_index find_attack_slice_insertion_pos(slice_index si, SliceType type);

/* Insert slices into a branch starting at a defense slice.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by insert_slices_defense_branch().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void insert_slices_defense_branch(slice_index si,
                                  slice_index const prototypes[],
                                  unsigned int nr_prototypes);

#endif
