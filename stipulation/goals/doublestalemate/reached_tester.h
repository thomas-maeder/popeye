#if !defined(STIPULATION_GOAL_DOUBLESTALEMATE_REACHED_TESTER_H)
#define STIPULATION_GOAL_DOUBLESTALEMATE_REACHED_TESTER_H

#include "pyslice.h"

/* This module provides functionality that detects whether double stalemate goal
 * has been reached
 */

/* Allocate a system of slices that test whether double stalemate has
 * been reached
 * @return index of entry slice
 */
slice_index alloc_goal_doublestalemate_reached_tester_system(void);

/* Instrument a doublestalemate testing system with a self check guard
 * @param tester STGoalReachedTester slice
 */
void goal_doublestalemate_insert_selfcheck_guard(slice_index tester);

#endif
