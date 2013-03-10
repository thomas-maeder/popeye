#if !defined(CONDITION_OWU_IMMOBILITY_TESTER_H)
#define CONDITION_OWU_IMMOBILITY_TESTER_H

#include "solving/solve.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Substitute OWU specific immobility testers
 * @param si where to start (entry slice into stipulation)
 */
void owu_replace_immobility_testers(slice_index si);

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
stip_length_type owu_immobility_tester_king_solve(slice_index si, stip_length_type n);

#endif
