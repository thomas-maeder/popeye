#if !defined(CONDITIONS_MARSCIRCE_ANTI_H)
#define CONDITIONS_MARSCIRCE_ANTI_H

#include "solving/machinery/solve.h"

/* This module provides implements the condition Anti-Mars-Circe
 */

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not have that walk
 */
void antimars_generate_moves_for_piece(slice_index si);

/* Inialise solving in Anti-Mars Circe
 */
void solving_initialise_antimars(slice_index si);

#endif
