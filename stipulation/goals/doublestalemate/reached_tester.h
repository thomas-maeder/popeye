#if !defined(STIPULATION_GOAL_DOUBLESTALEMATE_REACHED_TESTER_H)
#define STIPULATION_GOAL_DOUBLESTALEMATE_REACHED_TESTER_H

#include "solving/machinery/solve.h"

/* This module provides functionality that detects whether double stalemate goal
 * has been reached
 */

/* Allocate a system of slices that test whether double stalemate has
 * been reached
 * @return index of entry slice
 */
slice_index alloc_goal_doublestalemate_reached_tester_system(void);

#endif
