#if !defined(CONDITIONS_TRANSMUTING_KINGS_SUPER_H)
#define CONDITIONS_TRANSMUTING_KINGS_SUPER_H

#include "solving/solve.h"

/* This module implements the condition Super-transmuting kings */

/* the mummer logic is (ab)used to priorise transmuting king moves */
int len_supertransmuting_kings(square sq_departure,
                               square sq_arrival,
                               square sq_capture);

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
stip_length_type supertransmuting_kings_transmuter_solve(slice_index si,
                                                          stip_length_type n);

/* Instrument slices with move tracers
 * @param si identifies root slice of solving machinery
 * @param side for whom
 */
void supertransmuting_kings_initialise_solving(slice_index si, Side side);

#endif
