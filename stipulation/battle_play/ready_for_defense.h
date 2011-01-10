#if !defined(STIPULATION_BATTLE_PLAY_READY_FOR_DEFENSE_H)
#define STIPULATION_BATTLE_PLAY_READY_FOR_DEFENSE_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with the defending side
 * in STReadyForDefense stipulation slices.
 */

/* Allocate a STReadyForDefense defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_ready_for_defense_slice(stip_length_type length,
                                          stip_length_type min_length);

/* Traversal of the moves beyond a slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_ready_for_defense(slice_index si,
                                           stip_moves_traversal *st);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type ready_for_defense_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type ready_for_defense_has_solution(slice_index si);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
ready_for_defense_defend_in_n(slice_index si,
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
stip_length_type
ready_for_defense_can_defend_in_n(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_max_unsolvable);

#endif
