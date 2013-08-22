#if !defined(CONDITIONS_CIRCE_REX_INCLUSIVE_H)
#define CONDITIONS_CIRCE_REX_INCLUSIVE_H

/* This module implements Circe rex inclusive */

#include "py.h"

extern boolean circe_is_rex_inclusive;

/* Inialise solving in Circe rex inclusive
 * @param si identifies root slice of solving machinery
 */
void circe_rex_inclusive_initialise_solving(slice_index si);

#endif
