#if !defined(STIPULATION_GOAL_CHECK_REACHED_TESTER_H)
#define STIPULATION_GOAL_CHECK_REACHED_TESTER_H

#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with slices that detect
 * whether a check goal has just been reached
 */

/* Allocate a system of slices that tests whether check has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_check_reached_tester_system(void);

/* Allocate a STGoalCheckReachedTester slice.
 * @param starter_or_adversary is the starter or its adversary to be in check?
 * @return index of allocated slice
 */
slice_index alloc_goal_check_reached_tester_slice(goal_applies_to_starter_or_adversary starter_or_adversary);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type goal_check_reached_tester_attack(slice_index si, stip_length_type n);

#endif
