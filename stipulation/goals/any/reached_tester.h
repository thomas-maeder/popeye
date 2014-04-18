#if !defined(STIPULATION_GOAL_ANY_REACHED_TESTER_H)
#define STIPULATION_GOAL_ANY_REACHED_TESTER_H

#include "stipulation/stipulation.h"

/* This module provides functionality dealing with slices that detect
 * whether an any goal has just been reached
 */

/* Allocate a system of slices that tests whether any has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_any_reached_tester_system(void);

#endif
