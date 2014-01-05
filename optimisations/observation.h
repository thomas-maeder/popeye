#if !defined(OPTIMISATIONS_OBSERVATION_H)
#define OPTIMISATIONS_OBSERVATION_H

/* Implement various optimisations regarding the detection of square observation */

#include "stipulation/stipulation.h"
#include "solving/observation.h"
#include "position/position.h"
#include "utilities/boolean.h"

/* Don't look for observations using walks for which there are no pieces
 */
boolean dont_try_observing_with_non_existing_walk(slice_index si,
                                                  validator_id evaluate);
boolean dont_try_observing_with_non_existing_walk_both_sides(slice_index si,
                                                             validator_id evaluate);

boolean optimise_away_observations_by_queen_initialise(slice_index si,
                                                       validator_id evaluate);
boolean optimise_away_observations_by_queen(slice_index si, validator_id evaluate);
boolean undo_optimise_observation_by_queen(slice_index si);

/* Determine whether observations are trivially validated (allowing for some
 * optimisations)
 * @param side for which side?
 */
boolean is_observation_trivially_validated(Side side);

/* Optimise the square observation machinery if possible
 * @param si identifies the root slice of the solving machinery
 */
void optimise_is_square_observed(slice_index si);

#endif
