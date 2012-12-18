#if !defined(CONDITIONS_MARSCIRCE_PLUS_H)
#define CONDITIONS_MARSCIRCE_PLUS_H

#include "solving/solve.h"

/* This module provides implements the condition Echecs Plus
 */

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param side side for which to generate moves for
 * @param p indicates the walk according to which to generate moves
 * @param sq_departure departure square of moves to be generated
 * @note the piece on the departure square need not have that walk
 */
void plus_generate_moves(Side side, piece p, square sq_departure);

#endif
