#if !defined(CONDITIONS_PEPO_H)
#define CONDITIONS_PEPO_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* Implementation of condition Pepo
 */

/* Validate an observater according to Pepo
 * @return true iff the observation is valid
 */
boolean pepo_validate_observer(slice_index si);

/* Determine whether a particular piece of the moving side is observed
 * @param sq position of the piece
 * @return true iff the piece occupying square sq is observed by the opponent
 */
boolean pepo_is_moving_piece_observed(square sq);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void pepo_generate_moves_for_piece(slice_index si);

/* Inialise the solving machinery with Pepo
 * @param si identifies root slice of solving machinery
 */
void pepo_initialise_solving(slice_index si);

#endif
