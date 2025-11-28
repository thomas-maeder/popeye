#if !defined(CONDITIONS_IMMOBILIO_H)
#define CONDITIONS_IMMOBILIO_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* Implementation of condition Immobilio
 */

/* Validate an observater according to Immobilio
 * @return true iff the observation is valid
 */
boolean immobilio_validate_observer(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void immobilio_generate_moves_for_piece(slice_index si);

/* Inialise the solving machinery with Immobilio
 * @param si identifies root slice of solving machinery
 */
void immobilio_initialise_solving(slice_index si);

#endif
