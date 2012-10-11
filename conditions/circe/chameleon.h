#if !defined(CONDITIONS_CIRCE_CHAMELEON_H)
#define CONDITIONS_CIRCE_CHAMELEON_H

/* This module implements Chameleon Circe */

#include "solving/solve.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type chameleon_circe_adapt_reborn_piece_solve(slice_index si,
                                                          stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_chameleon_circe(slice_index si);

#endif
