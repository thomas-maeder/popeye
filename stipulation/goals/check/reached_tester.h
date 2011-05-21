#if !defined(STIPULATION_GOAL_CHECK_REACHED_TESTER_H)
#define STIPULATION_GOAL_CHECK_REACHED_TESTER_H

#include "pyslice.h"

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

/* Determine whether a slice has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_check_reached_tester_has_solution(slice_index si);

#endif
