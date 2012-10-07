#if !defined(STIPULATION_GOAL_ANY_REACHED_TESTER_H)
#define STIPULATION_GOAL_ANY_REACHED_TESTER_H

#include "solving/solve.h"

/* This module provides functionality dealing with slices that detect
 * whether an any goal has just been reached
 */

/* Allocate a system of slices that tests whether any has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_any_reached_tester_system(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type goal_any_reached_tester_solve(slice_index si, stip_length_type n);

#endif
