#if !defined(CONDITIONS_BEAMTEN_H)
#define CONDITIONS_BEAMTEN_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Beamtenschach */

/* Determine whether a Beamter piece is observed
 * @param sq_departure position of the piece
 * @return true iff the piece is observed, enabling it to move
 */
boolean beamten_is_observed(square sq_departure);

#endif
