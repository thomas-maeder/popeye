#if !defined(CONDITIONS_PATROL_H)
#define CONDITIONS_PATROL_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Patrol Chess */

/* Determine whether a pice is supported, enabling it to capture
 * @param sq_departure position of the piece
 * @return true iff the piece is supported
 */
boolean patrol_is_supported(square sq_departure);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type patrol_remove_unsupported_captures_solve(slice_index si,
                                                          stip_length_type n);

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_patrol(slice_index si);

#endif
