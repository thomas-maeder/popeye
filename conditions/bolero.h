#if !defined(CONDITIONS_BOLERO_H)
#define CONDITIONS_BOLERO_H

#include "stipulation/stipulation.h"

/* This module provides implements the condition Bolero
 */

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void bolero_generate_non_captures(slice_index si);

/* Inialise thet solving machinery with Mars Circe
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_bolero(slice_index si);

#endif
