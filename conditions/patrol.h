#if !defined(CONDITIONS_PATROL_H)
#define CONDITIONS_PATROL_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Patrol Chess */

/* Determine whether a pice is supported, enabling it to capture
 * @param sq_departure position of the piece
 * @return true iff the piece is supported
 */
boolean patrol_is_supported(square sq_departure);

#endif
