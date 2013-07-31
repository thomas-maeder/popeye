#if !defined(CONDITIONS_EIFFEL_H)
#define CONDITIONS_EIFFEL_H

#include "utilities/boolean.h"
#include "py.h"

/* Implementation of condition Eiffel chess
 */

/* Validate an observater according to Eiffel Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @return true iff the observation is valid
 */
boolean eiffel_validate_observer(slice_index si,
                                 square sq_observer,
                                 square sq_landing);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void eiffel_generate_moves_for_piece(slice_index si,
                                     square sq_departure,
                                     PieNam p);

/* Inialise the solving machinery with Eiffel Chess
 * @param si identifies root slice of solving machinery
 */
void eiffel_initialise_solving(slice_index si);

#endif
