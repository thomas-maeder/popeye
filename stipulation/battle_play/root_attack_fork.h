#if !defined(STIPULATION_BATTLE_PLAY_ROOT_ATTACK_FORK_H)
#define STIPULATION_BATTLE_PLAY_ROOT_ATTACK_FORK_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STRootAttackFork stipulation slices.
 */

/* Allocate a STRootAttackFork slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_next identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_root_attack_fork_slice(stip_length_type length,
                                         stip_length_type min_length,
                                         slice_index proxy_to_next);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void root_attack_fork_reduce_to_postkey_play(slice_index si,
                                             stip_structure_traversal *st);

/* Traversal of the moves beyond a series fork slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_root_attack_fork(slice_index si,
                                          stip_moves_traversal *st);

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
root_attack_fork_solve_in_n(slice_index si,
                            stip_length_type n,
                            stip_length_type n_max_unsolvable);

#endif
