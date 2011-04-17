#if !defined(OPTIMISATION_KILLER_MOVE_FINAL_DEFENSE_MOVE_H)
#define OPTIMISATION_KILLER_MOVE_FINAL_DEFENSE_MOVE_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with the defending side
 * in STKillerMoveFinalDefenseMove stipulation slices.
 * These provide an optimisation in stipulations where a defense move
 * leads to a goal (e.g. selfmate): if we have a "killer" piece, its
 * moves will be prefered when looking for refutations in the final
 * move.
 */

/* Reset the enabled state of the optimisation
 */
void reset_killer_move_final_defense_move_optimisation(void);

/* Disable the optimisation for defense moves by one side
 * @param side side for which to disable the optimisation
 */
void disable_killer_move_final_defense_move_optimisation(Side);

/* Optimise a STDefenseMoveGenerator slice for defending against a goal
 * @param si identifies slice to be optimised
 * @param goal goal that slice si defends against
 * @param full_length full length of branch
 */
void killer_move_optimise_final_defense_move(slice_index si,
                                             Goal goal,
                                             stip_length_type full_length);

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
killer_move_final_defense_move_can_defend(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable);

#endif
