#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_AGAINST_GOAL_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_MOVE_AGAINST_GOAL_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with the defending side
 * in STDefenseMoveAgainstGoal stipulation slices.
 */

/* Allocate a STDefenseMoveAgainstGoal defender slice.
 * @return index of allocated slice
 */
slice_index alloc_defense_move_against_goal_slice(void);

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
