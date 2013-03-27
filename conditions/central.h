#if !defined(CONDITIONS_CENTRAL_H)
#define CONDITIONS_CENTRAL_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Central Chess */

/* Determine whether a move is legal according to Central Chess
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture position of the capturee (if any)
 * @return true iff the move is legal
 */
boolean central_can_piece_move_from(square sq_departure);

/* Inialise solving in Central Chess
 */
void central_initialise_solving(void);

#endif
