#if !defined(CONDITIONS_SHIELDED_KINGS_H)
#define CONDITIONS_SHIELDED_KINGS_H

#include "py.h"

/* Implementation of condition Shielded kings
 */

/* Validate an observation according to Shielded Kings
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @return true iff the observation is valid
 */
boolean shielded_kings_validate_observation(slice_index si,
                                            square sq_observer,
                                            square sq_landing);

/* Inialise solving in Shielded kings
 * @param si identifies the root slice of the solving machinery
 */
void shielded_kings_initialise_solving(slice_index si);

#endif
