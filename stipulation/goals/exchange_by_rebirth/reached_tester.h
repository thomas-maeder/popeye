#if !defined(STIPULATION_GOAL_EXCHANGE_BY_REBIRTH_REACHED_TESTER_H)
#define STIPULATION_GOAL_EXCHANGE_BY_REBIRTH_REACHED_TESTER_H

#include "stipulation/stipulation.h"

/* This module provides functionality dealing with slices that detect
 * whether a exchange (by rebirth) goal has just been reached
 */

/* Allocate a system of slices that tests whether exchange_by_rebirth has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_exchange_by_rebirth_reached_tester_system(void);

#endif
