#if !defined(STIPULATION_GOAL_REACHED_TESTER_H)
#define STIPULATION_GOAL_REACHED_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a goal has just been reached
 */

/* Determine whether a side has reached the goal of a leaf slice.
 * @param just_moved side that has just moved
 * @param goal goal to be tested
 * @return whether just_moved has reached leaf's goal
 */
goal_checker_result_type is_goal_reached(Side just_moved, Goal goal);

/* Allocate a STGoalReachedTester slice.
 * @param Goal goal to be tested
 * @return index of allocated slice
 */
slice_index alloc_goal_reached_tester_slice(Goal goal);

/* Determine whether a slice has just been solved with the move
 * by the non-starter 
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_reached_tester_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_reached_tester_solve(slice_index si);

#endif
