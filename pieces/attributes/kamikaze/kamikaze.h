#if !defined(PIECES_KAMIKAZE_H)
#define PIECES_KAMIKAZE_H

/* This module implements Kamikaze pieces */

#include "solving/solve.h"


/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type kamikaze_capturing_piece_remover_solve(slice_index si,
                                                         stip_length_type n);

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_kamikaze(slice_index si);

#endif
