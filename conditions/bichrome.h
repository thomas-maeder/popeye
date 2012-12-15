#if !defined(CONDITIONS_BICHROME_H)
#define CONDITIONS_BICHROME_H

#include "py.h"

/* This module implements the condition Bichrome Chess */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type bichrome_remove_monochrome_moves_solve(slice_index si,
                                                        stip_length_type n);

/* Instrument the solvers with Bichrome Chess
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_bichrome(slice_index si);

#endif
