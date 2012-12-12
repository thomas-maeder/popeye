#if !defined(CONDITIONS_LORTAP_H)
#define CONDITIONS_LORTAP_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Lortap */

/* Determine whether a pice is supported, disabling it from capturing
 * @param sq_departure position of the piece
 * @return true iff the piece is supported
 */
boolean lortap_is_supported(square sq_departure);

#endif
