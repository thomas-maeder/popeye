#if !defined(STIPULATION_GOAL_AUTOSTALEMATE_REACHED_TESTER_H)
#define STIPULATION_GOAL_AUTOSTALEMATE_REACHED_TESTER_H

#include "pystip.h"

/* This module provides functionality dealing with slices that detect
 * whether a auto-stalemate goal has just been reached
 */

/* Allocate a system of slices that tests whether autostalemate has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_autostalemate_reached_tester_system(void);

#endif
