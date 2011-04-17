#if !defined(OPTIMISATION_KILLER_MOVE_ATTACK_GENERATOR_H)
#define OPTIMISATION_KILLER_MOVE_ATTACK_GENERATOR_H

#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with the attacking side
 * in STKillerMoveAttackGenerator stipulation slices.
 */

/* Allocate a STKillerMoveAttackGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_killer_move_attack_generator_slice(void);

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
killer_move_attack_generator_can_attack(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_max_unsolvable);

#endif
