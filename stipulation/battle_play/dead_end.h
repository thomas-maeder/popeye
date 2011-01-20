#if !defined(STIPULATION_BATTLE_PLAY_DEAD_END_H)
#define STIPULATION_BATTLE_PLAY_DEAD_END_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with the defending side
 * in STDefenseMove stipulation slices.
 */

/* Allocate a STBattleDeadEnd defender slice.
 * @return index of allocated slice
 */
slice_index alloc_battle_play_dead_end_slice(void);

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
battle_play_dead_end_defend_in_n(slice_index si,
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
battle_play_dead_end_can_defend_in_n(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_max_unsolvable);

#endif
