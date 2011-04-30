#if !defined(STIPULATION_BATTLE_PLAY_END_OF_BRANCH_H)
#define STIPULATION_BATTLE_PLAY_END_OF_BRANCH_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with the defending side
 * in STEndOfBattleBranch stipulation slices.
 */

/* Allocate a STEndOfBattleBranch slice.
 * @param proxy_to_next identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_end_of_battle_branch_slice(slice_index proxy_to_next);

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void end_of_battle_branch_make_root(slice_index si,
                                    stip_structure_traversal *st);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void end_of_battle_branch_reduce_to_postkey_play(slice_index si,
                                                 stip_structure_traversal *st);

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_end_of_branch(slice_index si,
                                           stip_structure_traversal *st);

/* Traversal of the moves beyond a series fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_end_of_battle_branch(slice_index si,
                                              stip_moves_traversal *st);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type end_of_battle_branch_defend(slice_index si,
                                             stip_length_type n,
                                             stip_length_type n_max_unsolvable);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
           n+2 refuted - <=acceptable number of refutations found
           n+4 refuted - >acceptable number of refutations found
 */
stip_length_type end_of_battle_branch_can_defend(slice_index si,
                                                 stip_length_type n,
                                                 stip_length_type n_max_unsolvable);

#endif
