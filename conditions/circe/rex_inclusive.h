#if !defined(CONDITIONS_CIRCE_REX_INCLUSIVE_H)
#define CONDITIONS_CIRCE_REX_INCLUSIVE_H

/* This module implements Circe rex inclusive */

#include "utilities/boolean.h"
#include "position/board.h"
#include "py.h"

/* Validate an observation according to Circe rex. incl.
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean circe_rex_inclusive_validate_observation(slice_index si,
                                                 square sq_observer,
                                                 square sq_landing,
                                                 square sq_observee);

/* Inialise solving in Circe rex inclusive
 * @param si identifies root slice of solving machinery
 */
void circe_rex_inclusive_initialise_solving(slice_index si);

#endif
