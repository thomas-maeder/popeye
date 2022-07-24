#if !defined(CONDITIONS_BOLERO_H)
#define CONDITIONS_BOLERO_H

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* This module provides implements the condition Bolero
 */

extern boolean bolero_is_rex_inclusive;

/* Make sure that the observer has the expected walk - annanised or originally
 * @return true iff the observation is valid
 */
boolean bolero_inverse_enforce_observer_walk(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void bolero_generate_moves(slice_index si);

/* Inialise thet solving machinery with Mars Circe
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_bolero(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void bolero_inverse_generate_captures(slice_index si);

/* Inialise thet solving machinery with Mars Circe
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_bolero_inverse(slice_index si);

#endif
