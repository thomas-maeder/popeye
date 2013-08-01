#if !defined(CONDITIONS_CENTRAL_H)
#define CONDITIONS_CENTRAL_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Central Chess */

/* Validate an observation according to Central Chess
 * @return true iff the observation is valid
 */
boolean central_validate_observation(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void central_generate_moves_for_piece(slice_index si,
                                      square sq_departure,
                                      PieNam p);

/* Inialise the solving machinery with Central Chess
 * @param si identifies root slice of solving machinery
 */
void central_initialise_solving(slice_index si);

#endif
