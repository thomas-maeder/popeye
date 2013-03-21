#if !defined(CONDITIONS_CENTRAL_H)
#define CONDITIONS_CENTRAL_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Central Chess */

/* Validate an observation according to Central Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean central_validate_observation(square sq_observer,
                                     square sq_landing,
                                     square sq_observee);

/* Determine whether a move is legal according to Central Chess
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture position of the capturee (if any)
 * @return true iff the move is legal
 */
boolean central_can_piece_move_from(square sq_departure);

#endif
