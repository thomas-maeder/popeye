#if !defined(CONDITION_MAFF_IMMOBILITY_TESTER_H)
#define CONDITION_MAFF_IMMOBILITY_TESTER_H

#include "solving/solve.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Replace immobility tester slices to cope with condition MAFF
 * @param si where to start (entry slice into stipulation)
 */
void maff_replace_immobility_testers(slice_index si);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type maff_immobility_tester_king_solve(slice_index si, stip_length_type n);

#endif
