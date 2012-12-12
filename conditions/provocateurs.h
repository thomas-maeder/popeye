#if !defined(CONDITIONS_PROVOCATEURS_H)
#define CONDITIONS_PROVOCATEURS_H

#include "solving/solve.h"

/* This module implements the condition Provocation Chess */

/* Determine whether a piece is observed
 * @param sq_departure position of the piece
 * @return true iff the piece is observed
 */
boolean provocateurs_is_observed(square sq_departure);

#endif
