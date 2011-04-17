#if !defined(STIPULATION_BATTLE_PLAY_ATTACK_MOVE_GENERATOR_H)
#define STIPULATION_BATTLE_PLAY_ATTACK_MOVE_GENERATOR_H

#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with the attacking side
 * in STAttackMoveGenerator stipulation slices.
 */

/* Allocate a STAttackMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_attack_move_generator_slice(void);

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
attack_move_generator_can_attack(slice_index si,
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
stip_length_type attack_move_generator_attack(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_max_unsolvable);

#endif
