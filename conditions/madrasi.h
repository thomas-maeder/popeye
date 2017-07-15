#if !defined(CONDITIONS_MADRASI_H)
#define CONDITIONS_MADRASI_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* Implementation of condition Madrasi
 */

extern boolean madrasi_is_rex_inclusive;

/* Determine whether a piece is observed by an opponent's piece with a certain
 * walk.
 * @param sq_observee position of the potentially observed piece
 * @param walk we are interested in observations of pieces with this walk
 * @param observed_side we are interested in observations by the opponent
 * @return true iff there is >=1 matchin observation
 */
boolean madrasi_is_piece_observed_by_walk(square sq_observee,
                                          piece_walk_type walk,
                                          Side observed_side);

/* Validate an observater according to Madrasi
 * @return true iff the observation is valid
 */
boolean madrasi_validate_observer(slice_index si);

/* Determine whether a particular piece of the moving side is observed
 * @param sq position of the piece
 * @return true iff the piece occupying square sq is observed by the opponent
 */
boolean madrasi_is_moving_piece_observed(square sq);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void madrasi_generate_moves_for_piece(slice_index si);

/* Inialise the solving machinery with Madrasi
 * @param si identifies root slice of solving machinery
 */
void madrasi_initialise_solving(slice_index si);

#endif
