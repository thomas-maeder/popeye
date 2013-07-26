#if !defined(CONDITIONS_BRUNNER_H)
#define CONDITIONS_BRUNNER_H

#include "py.h"

/* Validate an observation according to Brunner Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean brunner_validate_observation(slice_index si,
                                     square sq_observer,
                                     square sq_landing,
                                     square sq_observee);

/* Inialise solving in Brunner Chess
 * @param si identifies the root slice of the solving machinery
 */
void brunner_initialise_solving(slice_index si);

#endif
