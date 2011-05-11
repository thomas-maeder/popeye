#if !defined(STIPULATION_GOAL_MATE_REACHED_TESTER_H)
#define STIPULATION_GOAL_MATE_REACHED_TESTER_H

#include "pyslice.h"

/* This module provides functionality that detects whether mate goal has
 * been reached
 */

/* Allocate a system of slices that tests whether mate has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_mate_reached_tester_system(void);

#endif
