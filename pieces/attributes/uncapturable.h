#if !defined(PIECES_ATTRIBUTES_HURDLE_UNCAPTURABLE_H)
#define PIECES_ATTRIBUTES_HURDLE_UNCAPTURABLE_H

#include "py.h"

/* This module implements Uncapturable pieces */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type uncapturable_remove_captures_solve(slice_index si,
                                                    stip_length_type n);

/* Instrument the solvers with support for Uncapturable pieces
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_uncapturable(slice_index si);

#endif
