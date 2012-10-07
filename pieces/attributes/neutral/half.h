#if !defined(PIECES_NEUTRAL_HALF_H)
#define PIECES_NEUTRAL_HALF_H

#include "solving/solve.h"

/* This module implements half-neutral pieces */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type half_neutral_recolorer_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_half_neutral_recolorers(slice_index si);

#endif
