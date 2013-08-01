#if !defined(CONDITIONS_SHIELDED_KINGS_H)
#define CONDITIONS_SHIELDED_KINGS_H

#include "py.h"

/* Implementation of condition Shielded kings
 */

/* Validate an observation according to Shielded Kings
 * @return true iff the observation is valid
 */
boolean shielded_kings_validate_observation(slice_index si);

/* Inialise solving in Shielded kings
 * @param si identifies the root slice of the solving machinery
 */
void shielded_kings_initialise_solving(slice_index si);

#endif
