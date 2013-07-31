#if !defined(CONDITIONS_MADRASI_H)
#define CONDITIONS_MADRASI_H

#include "utilities/boolean.h"
#include "py.h"

/* Implementation of condition Madrasi
 */

/* Validate an observater according to Madrasi
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @return true iff the observation is valid
 */
boolean madrasi_validate_observer(slice_index si,
                                  square sq_observer,
                                  square sq_landing);

/* Determine whether a particular piece of the moving side is observed
 * @param sq position of the piece
 * @return true iff the piece occupying square sq is observed by the opponent
 */
boolean madrasi_is_moving_piece_observed(square sq);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void madrasi_generate_moves_for_piece(slice_index si,
                                      square sq_departure,
                                      PieNam p);

/* Inialise the solving machinery with Madrasi
 * @param si identifies root slice of solving machinery
 */
void madrasi_initialise_solving(slice_index si);

#endif
