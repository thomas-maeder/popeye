#if !defined(STIPULATION_GOAL_ANY_REACHED_TESTER_H)
#define STIPULATION_GOAL_ANY_REACHED_TESTER_H

#include "pystip.h"

/* This module provides functionality dealing with slices that detect
 * whether an any goal has just been reached
 */

/* Allocate a STGoalAnyReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_any_reached_tester_slice(void);

#endif
