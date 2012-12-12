#if !defined(CONDITIONS_LORTAP_H)
#define CONDITIONS_LORTAP_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Lortap */

/* Determine whether a pice is supported, disabling it from capturing
 * @param sq_departure position of the piece
 * @return true iff the piece is supported
 */
boolean lortap_is_supported(square sq_departure);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type lortap_remove_supported_captures_solve(slice_index si,
                                                        stip_length_type n);

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_lortap(slice_index si);

#endif
