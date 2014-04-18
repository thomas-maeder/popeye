#if !defined(STIPULATION_GOAL_ATOB_REACHED_TESTER_H)
#define STIPULATION_GOAL_ATOB_REACHED_TESTER_H

#include "solving/machinery/solve.h"

/* This module provides functionality dealing with slices that detect
 * whether an A=>B game goal has just been reached
 */

/* Allocate a system of slices that tests whether atob has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_atob_reached_tester_system(void);

#endif
