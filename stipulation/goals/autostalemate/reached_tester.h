#if !defined(STIPULATION_GOAL_AUTOSTALEMATE_REACHED_TESTER_H)
#define STIPULATION_GOAL_AUTOSTALEMATE_REACHED_TESTER_H

#include "pystip.h"

/* This module provides functionality dealing with slices that detect
 * whether a auto-stalemate goal has just been reached
 */

/* Allocate a STGoalAutoStalemateReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_autostalemate_reached_tester_slice(void);

#endif
