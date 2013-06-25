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

/* Determine whether a side observes a specific square
 * @param side_observing the side
 * @param sq_target square potentially observed
 * @return true iff side is in check
 */
boolean plus_is_square_observed(Side side_observing,
                                square sq_target,
                                evalfunction_t *evaluate);

#endif
