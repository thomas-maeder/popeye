#if !defined(STIPULATION_GOAL_KING_CAPTURE_REACHED_TESTER_H)
#define STIPULATION_GOAL_KING_CAPTURE_REACHED_TESTER_H

#include "stipulation/stipulation.h"

/* This module provides functionality dealing with slices that detect
 * whether king capture has just been reached
 */

/* Allocate a system of slices that tests whether king capture has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_king_capture_reached_tester_system(void);

#endif
