#if !defined(STIPULATION_GOAL_STALEMATE_REACHED_TESTER_H)
#define STIPULATION_GOAL_STALEMATE_REACHED_TESTER_H

#include "solving/machinery/solve.h"

/* This module provides functionality that detects whether stalemate goal has
 * been reached
 */

/* Allocate a system of slices that tests whether mate has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_stalemate_reached_tester_system(void);

#endif
