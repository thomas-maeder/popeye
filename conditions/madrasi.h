#if !defined(CONDITIONS_MADRASI_H)
#define CONDITIONS_MADRASI_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* Implementation of condition Madrasi
 */

extern boolean madrasi_is_rex_inclusive;

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
 * @param p walk to be used for generating
 */
void madrasi_generate_moves_for_piece(slice_index si, PieNam p);

/* Inialise the solving machinery with Madrasi
 * @param si identifies root slice of solving machinery
 */
void madrasi_initialise_solving(slice_index si);

#endif
