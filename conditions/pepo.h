#if !defined(CONDITIONS_PEPO_H)
#define CONDITIONS_PEPO_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* Implementation of condition Pepo
 */

/* Determine whether a square is observed be the side at the move according to
 * Pepo
 * @param si identifies next slice
 * @note sets observation_result
 */
void pepo_is_square_observed(slice_index si);

boolean pepo_check_test_initialiser_is_in_check(slice_index si,
                                                Side side_king_attacked);

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
