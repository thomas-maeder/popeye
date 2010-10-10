#if !defined(STIPULATION_GOAL_ATOB_REACHED_TESTER_H)
#define STIPULATION_GOAL_ATOB_REACHED_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether an A=>B game goal has just been reached
 */

/* Allocate a STGoalAToBReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_atob_reached_tester_slice(void);

#endif
