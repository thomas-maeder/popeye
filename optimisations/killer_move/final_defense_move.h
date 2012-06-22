#if !defined(OPTIMISATION_KILLER_MOVE_FINAL_DEFENSE_MOVE_H)
#define OPTIMISATION_KILLER_MOVE_FINAL_DEFENSE_MOVE_H

#include "solving/battle_play/defense_play.h"

/* This module provides functionality dealing with the defending side
 * in STKillerMoveFinalDefenseMove stipulation slices.
 * These provide an optimisation in stipulations where a defense move
 * leads to a goal (e.g. selfmate): if we have a "killer" piece, its
 * moves will be prefered when looking for refutations in the final
 * move.
 */

/* Optimise a STMoveGenerator slice for defending against a goal
 * @param si identifies slice to be optimised
 */
void killer_move_optimise_final_defense_move(slice_index si);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type killer_move_final_defense_move_defend(slice_index si,
                                                       stip_length_type n);

#endif
