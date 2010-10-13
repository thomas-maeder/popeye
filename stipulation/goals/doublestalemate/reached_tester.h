#if !defined(STIPULATION_GOAL_DOUBLESTALEMATE_REACHED_TESTER_H)
#define STIPULATION_GOAL_DOUBLESTALEMATE_REACHED_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a double stalemate goal has just been reached
 */

/* Allocate a STGoalDoubleStalemateReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_doublestalemate_reached_tester_slice(void);

#endif
