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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type patrol_remove_unsupported_captures_solve(slice_index si,
                                                          stip_length_type n);

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_patrol(slice_index si);

#endif
