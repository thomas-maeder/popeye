#if !defined(STIPULATION_GOAL_CIRCUIT_REACHED_TESTER_H)
#define STIPULATION_GOAL_CIRCUIT_REACHED_TESTER_H

#include "stipulation/stipulation.h"

/* This module provides functionality dealing with slices that detect
 * whether a circuit goal has just been reached
 */

/* Allocate a system of slices that tests whether circuit has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_circuit_reached_tester_system(void);

#endif
