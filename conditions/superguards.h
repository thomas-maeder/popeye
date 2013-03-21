#if !defined(CONDITIONS_SUPERGUARDS_H)
#define CONDITIONS_SUPERGUARDS_H

#include "py.h"

/* This module implements the condition Superguards */

/* Validate an observation according to Superguards
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean superguards_validate_observation(square sq_departure,
                                         square sq_arrival,
                                         square sq_capture);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type superguards_remove_illegal_captures_solve(slice_index si,
                                                           stip_length_type n);

/* Instrument the solvers with Superguards
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_superguards(slice_index si);

#endif
