#if !defined(CONDITIONS_EIFFEL_H)
#define CONDITIONS_EIFFEL_H

#include "py.h"
#include "pyproc.h"

/* Implementation of condition Eiffel chess
 */

/* Can a piece on a particular square can move according to Eiffel chess?
 * @param sq position of piece
 * @return true iff the piece can move according to Disparate chess
 */
boolean eiffel_can_piece_move(square sq);

/* Can a piece deliver check according to Eiffel chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean eiffel_validate_observation(square sq_observer,
                                    square sq_landing,
                                    square sq_observee);

#endif
