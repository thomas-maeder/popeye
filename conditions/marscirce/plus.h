#if !defined(CONDITIONS_MARSCIRCE_PLUS_H)
#define CONDITIONS_MARSCIRCE_PLUS_H

#include "solving/solve.h"
#include "pyproc.h"

/* This module provides implements the condition Echecs Plus
 */

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param side side for which to generate moves for
 * @param p indicates the walk according to which to generate moves
 * @param sq_departure departure square of moves to be generated
 * @note the piece on the departure square need not have that walk
 */
void plus_generate_moves(Side side, PieNam p, square sq_departure);

/* Determine whether a specific side is in check in Echecs Plus
 * @param side the side
 * @param evaluate filter for king capturing moves
 * @return true iff side is in check
 */
boolean plusechecc(Side side, evalfunction_t *evaluate);

#endif
