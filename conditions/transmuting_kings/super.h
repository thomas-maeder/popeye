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
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type supertransmuting_kings_transmuter_solve(slice_index si,
                                                          stip_length_type n);

/* Instrument slices with move tracers
 */
void stip_insert_supertransmuting_kings(slice_index si);

#endif
