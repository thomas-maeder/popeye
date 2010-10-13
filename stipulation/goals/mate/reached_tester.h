#if !defined(STIPULATION_GOAL_MATE_REACHED_TESTER_H)
#define STIPULATION_GOAL_MATE_REACHED_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a mate goal has just been reached
 */

/* Allocate a STGoalMateReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_mate_reached_tester_slice(void);

#endif
