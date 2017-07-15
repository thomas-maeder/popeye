#if !defined(CONDITIONS_PARTIAL_PARALYSIS_H)
#define CONDITIONS_PARTIAL_PARALYSIS_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* Implementation of condition Partial Paralysis
 */

/* Validate an observater according to Partial Paralysis
 * @return true iff the observation is valid
 */
boolean partial_paralysis_validate_observer(slice_index si);

/* Determine whether a particular piece of the moving side is observed
 * @param sq position of the piece
 * @return true iff the piece occupying square sq is observed by the opponent
 */
boolean partial_paralysis_is_moving_piece_observed(square sq);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void partial_paralysis_generate_moves_for_piece(slice_index si);

/* Inialise the solving machinery with Partial Paralysis
 * @param si identifies root slice of solving machinery
 */
void partial_paralysis_initialise_solving(slice_index si);

#endif
