#if !defined(CONDITIONS_BRUNNER_H)
#define CONDITIONS_BRUNNER_H

#include "stipulation/stipulation.h"

/* Validate an observation according to Brunner Chess
 * @return true iff the observation is valid
 */
boolean brunner_validate_observation(slice_index si);

/* Inialise solving in Brunner Chess
 * @param si identifies the root slice of the solving machinery
 */
void brunner_initialise_solving(slice_index si);

#endif
