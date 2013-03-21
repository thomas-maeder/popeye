#if !defined(CONDITIONS_BEAMTEN_H)
#define CONDITIONS_BEAMTEN_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Beamtenschach */

/* Determine whether a Beamter piece is observed
 * @param sq_departure position of the piece
 * @return true iff the piece is observed, enabling it to move
 */
boolean beamten_is_observed(square sq_departure);

/* Validate an observation according to Beamten Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean beamten_validate_observation(square sq_observer,
                                     square sq_landing,
                                     square sq_observee);

#endif
