#if !defined(CONDITIONS_MARSCIRCE_ANTI_H)
#define CONDITIONS_MARSCIRCE_ANTI_H

#include "solving/machinery/solve.h"
#include "conditions/circe/circe.h"

/* This module provides implements the condition Anti-Mars-Circe
 */

extern circe_variant_type antimars_variant;

/* Prevent Anti-Marscirce from generating null moves
 * @param si identifies the slice
 */
void anti_mars_circe_reject_null_moves(slice_index si);

/* Inialise solving in Anti-Mars Circe
 */
void solving_initialise_antimars(slice_index si);

#endif
