#if !defined(CONDITIONS_CENTRAL_H)
#define CONDITIONS_CENTRAL_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Central Chess */

/* Determine whether a pice is supported
 * @param sq_departure position of the piece
 * @return true iff the piece is supported
 */
boolean central_is_supported(square sq_departure);

#endif
