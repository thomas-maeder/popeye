#if !defined(CONDITIONS_MARSCIRCE_PHANTOM_H)
#define CONDITIONS_MARSCIRCE_PHANTOM_H

#include "pieces/pieces.h"
#include "solving/machinery/solve.h"
#include "solving/observation.h"

/* This module provides implements the condition Phantom Chess
 */

extern boolean phantom_chess_rex_inclusive;

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void phantom_enforce_rex_inclusive(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void phantom_avoid_duplicate_moves(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not have that walk
 */
void phantom_generate_moves_for_piece(slice_index si);

/* Inialise the solving machinery with Phantom Chess
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_phantom(slice_index si);

/* Determine whether a specific side is in check in Phantom Chess
 * @param si identifies tester slice
 * @return true iff side is in check
 */
boolean phantom_is_square_observed(slice_index si, validator_id evaluate);

#endif
