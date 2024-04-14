#if !defined(CONDITIONS_EIFFEL_H)
#define CONDITIONS_EIFFEL_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* Implementation of condition Eiffel chess
 */

/* Determine whether a particular piece is observed
 * @param pos position of the piece
 * @return true iff the piece occupying square pos is observed by the opponent
 */
boolean eiffel_is_piece_observed(square pos);

/* Validate an observater according to Eiffel Chess
 * @return true iff the observation is valid
 */
boolean eiffel_validate_observer(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void eiffel_generate_moves_for_piece(slice_index si);

/* Inialise the solving machinery with Eiffel Chess
 * @param si identifies root slice of solving machinery
 */
void eiffel_initialise_solving(slice_index si);

#endif
