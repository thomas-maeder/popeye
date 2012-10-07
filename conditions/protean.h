#if !defined(CONDITIONS_PROTEAN_H)
#define CONDITIONS_PROTEAN_H

#include "solving/solve.h"

/* This module implements the condition Protean Chess */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type protean_pawn_adjuster_solve(slice_index si,
                                              stip_length_type n);

/* Instrument slices with move tracers
 */
void stip_insert_protean_chess(slice_index si);

#endif
