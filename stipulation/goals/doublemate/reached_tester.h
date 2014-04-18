#if !defined(STIPULATION_GOAL_DOUBLEMATE_REACHED_TESTER_H)
#define STIPULATION_GOAL_DOUBLEMATE_REACHED_TESTER_H

#include "stipulation/stipulation.h"

/* This module provides functionality dealing with slices that detect
 * whether a double mate goal has just been reached
 */

/* Allocate a system of slices that tests whether doublemate has been reached
 * @return index of entry slice
 */
slice_index alloc_doublemate_mate_reached_tester_system(void);

#endif
