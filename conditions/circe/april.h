#if !defined(CONDITIONS_CIRCE_APRIL_H)
#define CONDITIONS_CIRCE_APRIL_H

#include "solving/solve.h"

/* This module implements April Chess */

extern boolean is_april_kind[PieceCount];

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type april_chess_fork_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation for strict Circe rebirths
 * @param si identifies root slice of stipulation
 */
void stip_insert_april_chess(slice_index si);

#endif
