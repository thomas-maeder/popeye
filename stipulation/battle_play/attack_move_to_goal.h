#if !defined(STIPULATION_BATTLE_PLAY_ATTACK_MOVE_TO_GOAL_H)
#define STIPULATION_BATTLE_PLAY_ATTACK_MOVE_TO_GOAL_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STAttackMoveToGoal stipulation slices.
 */

/* Allocate a STAttackMoveToGoal slice.
 * @param goal goal to be reached
 * @return index of allocated slice
 */
slice_index alloc_attack_move_to_goal_slice(Goal goal);

/* Optimise a STAttackMove slice
 * @param si identifies slice to be optimised
 * @param goal goal that slice si attempts to reach
 */
void optimise_final_attack_move(slice_index si, Goal goal);

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
attack_move_to_goal_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
attack_move_to_goal_solve_in_n(slice_index si,
                               stip_length_type n,
                               stip_length_type n_max_unsolvable);

#endif
