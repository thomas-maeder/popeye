#if !defined(STIPULATION_BATTLE_PLAY_ATTACK_FIND_SHORTEST_H)
#define STIPULATION_BATTLE_PLAY_ATTACK_FIND_SHORTEST_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STAttackFindShortest stipulation slices.
 */

/* Allocate a STAttackFindShortest slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_attack_find_shortest_slice(stip_length_type length,
                                             stip_length_type min_length);

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void attack_find_shortest_make_root(slice_index si,
                                    stip_structure_traversal *st);

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximal number of moves
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
attack_find_shortest_can_attack(slice_index si,
                                stip_length_type n,
                                stip_length_type n_max_unsolvable);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
attack_find_shortest_solve_in_n(slice_index si,
                                stip_length_type n,
                                stip_length_type n_max_unsolvable);


#endif
