#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_END_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_END_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with the defending side
 * in STDefenseEnd stipulation slices.
 */

/* Allocate a STDefenseEnd defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_end_slice(stip_length_type length,
                                    stip_length_type min_length);

/* Traversal of the moves beyond a series fork slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_defense_end(slice_index si, stip_moves_traversal *st);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void defense_end_reduce_to_postkey_play(slice_index si,
                                        stip_structure_traversal *st);

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void defense_end_make_root(slice_index si, stip_structure_traversal *st);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type defense_end_defend_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 (incl. defense) needed
 n+2 refuted - <=max_nr_refutations refutations found
 n+4 refuted - >max_nr_refutations refutations found
*/
stip_length_type
defense_end_can_defend_in_n(slice_index si,
                            stip_length_type n,
                            stip_length_type n_max_unsolvable,
                            unsigned int max_nr_refutations);

#endif
