#if !defined(CONDITIONS_CIRCE_REX_INCLUSIVE_H)
#define CONDITIONS_CIRCE_REX_INCLUSIVE_H

#include "pyproc.h"

/* This module implements Circe rex inclusive */

/* Validate an observation according to Circe rex inclusive
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean circe_rex_inclusive_validate_observation(square sq_observer,
                                                 square sq_landing,
                                                 square sq_observee);

#endif
