#if !defined(CONDITIONS_MADRASI_H)
#define CONDITIONS_MADRASI_H

#include "py.h"
#include "pyproc.h"

/* Implementation of condition Madrasi
 */

boolean madrasi_is_observed(square sq);

/* Can a piece on a particular square can move according to Madrasi?
 * @param sq position of piece
 * @return true iff the piece can move according to Madrasi
 */
boolean madrasi_can_piece_move(square sq);

/* Can a piece deliver check according to Madrasi
 * @param sq_departure position of the piece
 * @param sq_arrival arrival square of the capture to be threatened
 * @param sq_capture typically the position of the opposite king
 */
boolean madrasi_validate_observation(square sq_observer,
                                     square sq_landing,
                                     square sq_observee);

#endif
