#if !defined(OPTIMISATION_KILLER_MOVE_ATTACK_GENERATOR_H)
#define OPTIMISATION_KILLER_MOVE_ATTACK_GENERATOR_H

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with the attacking side
 * in STKillerMoveMoveGenerator stipulation slices.
 */

/* Allocate a STKillerMoveMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_killer_move_move_generator_slice(void);

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximal number of moves
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
killer_move_move_generator_can_attack(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
killer_move_move_generator_defend(slice_index si, stip_length_type n);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
killer_move_move_generator_can_defend(slice_index si, stip_length_type n);

#endif
