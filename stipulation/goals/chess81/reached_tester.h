#if !defined(STIPULATION_GOAL_CHESS81_REACHED_TESTER_H)
#define STIPULATION_GOAL_CHESS81_REACHED_TESTER_H

#include "solving/battle_play/attack_play.h"

/* This module provides functionality dealing with slices that detect
 * whether an chess81 goal has just been reached
 */

/* Allocate a system of slices that tests whether first row or last row has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_chess81_reached_tester_system(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type goal_chess81_reached_tester_attack(slice_index si, stip_length_type n);

#endif
