#if !defined(CONDITIONS_DISPARATE_H)
#define CONDITIONS_DISPARATE_H

#include "py.h"

/* Implementation of condition Disparate chess
 */

/* Can a piece on a particular square can move according to Disparate chess?
 * @param sq position of piece
 * @return true iff the piece can move according to Disparate chess
 */
boolean disparate_can_piece_move(square sq);

/* Validate an observation according to Disparate chess
 * @param sq_departure position of the piece
 * @param sq_arrival arrival square of the capture to be threatened
 * @param sq_capture typically the position of the opposite king
 */
boolean disparate_validate_observation(square sq_observer,
                                       square sq_landing,
                                       square sq_observee);

#endif
