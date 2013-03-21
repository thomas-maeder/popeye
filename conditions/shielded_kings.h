#if !defined(CONDITIONS_SHIELDED_KINGS_H)
#define CONDITIONS_SHIELDED_KINGS_H

#include "py.h"

/* Implementation of condition Shielded kings
 */

/* Can a piece deliver check according to Shielded kings
 * @param sq_departure position of the piece
 * @param sq_arrival arrival square of the capture to be threatened
 * @param sq_capture typically the position of the opposite king
 */
boolean shielded_kings_validate_observation(square sq_observer,
                                            square sq_landing,
                                            square sq_observee);

#endif
