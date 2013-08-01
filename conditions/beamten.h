#if !defined(CONDITIONS_BEAMTEN_H)
#define CONDITIONS_BEAMTEN_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Beamtenschach */

/* Validate an observation according to Beamten Chess
 * @return true iff the observation is valid
 */
boolean beamten_validate_observation(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void beamten_generate_moves_for_piece(slice_index si,
                                      square sq_departure,
                                      PieNam p);

/* Inialise the solving machinery with Beamten Chess
 * @param si identifies root slice of solving machinery
 */
void beamten_initialise_solving(slice_index si);

#endif
