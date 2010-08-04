#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_AGAINST_GOAL_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_AGAINST_GOAL_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with the defending side
 * in STDefenseMoveAgainstGoal stipulation slices.
 */

/* Reset the enabled state of the optimisation of final defense moves
 */
void reset_defense_move_against_goal_enabled_state(void);

/* Disable the optimisation of final defense moves for defense by a side
 * @param side side for which to disable the optimisation
 */
void disable_defense_move_against_goal(Side);

/* Optimise a STDefenseMove slice for defending against a goal
 * @param si identifies slice to be optimised
 * @param goal goal that slice si defends against
 * @return index of allocated slice
 */
void optimise_final_defense_move(slice_index si, Goal goal);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
           n+2 refuted - <=max_nr_refutations refutations found
           n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
defense_move_against_goal_can_defend_in_n(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable,
                                          unsigned int max_nr_refutations);

#endif
