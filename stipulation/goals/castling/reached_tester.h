#if !defined(STIPULATION_GOAL_CASTLING_REACHED_TESTER_H)
#define STIPULATION_GOAL_CASTLING_REACHED_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a castling goal has just been reached
 */

/* Allocate a STGoalCastlingReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_castling_reached_tester_slice(void);

/* Determine whether a slice has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_castling_reached_tester_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_castling_reached_tester_solve(slice_index si);

#endif
