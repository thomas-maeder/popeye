#if !defined(STIPULATION_GOAL_CIRCUIT_REACHED_TESTER_H)
#define STIPULATION_GOAL_CIRCUIT_REACHED_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a circuit goal has just been reached
 */

/* Allocate a system of slices that tests whether circuit has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_circuit_reached_tester_system(void);

/* Determine whether a slice has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_circuit_reached_tester_has_solution(slice_index si);

#endif
