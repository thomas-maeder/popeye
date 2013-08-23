#if !defined(OPTIONS_GOAL_IS_END_H)
#define OPTIONS_GOAL_IS_END_H

/* This module implements option GoalIsEnd
 */

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* Instrument a stipulation for option GoalIsEnd
 * @param root_slice identifies root slice of stipulation
 * @return true iff option GoalIsEnd is applicable
 */
boolean stip_insert_goal_is_end_testers(slice_index root_slice);

#endif
