#if !defined(STIPULATION_GOAL_CIRCUIT_BY_REBIRTH_REACHED_TESTER_H)
#define STIPULATION_GOAL_CIRCUIT_BY_REBIRTH_REACHED_TESTER_H

#include "solving/solve.h"

/* This module provides functionality dealing with slices that detect
 * whether a circuit (by rebirth) goal has just been reached
 */

/* Allocate a system of slices that tests whether circuit_by_rebirth has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_circuit_by_rebirth_reached_tester_system(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type
goal_circuit_by_rebirth_reached_tester_solve(slice_index si,
                                              stip_length_type n);

#endif
