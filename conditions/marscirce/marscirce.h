#if !defined(CONDITIONS_MARSCIRCE_MARSCIRCE_H)
#define CONDITIONS_MARSCIRCE_MARSCIRCE_H

#include "solving/solve.h"
#include "pyproc.h"

/* This module provides implements the condition Mars-Circe
 */

/* Generate non-capturing moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 * @param sq_real_departure real departure square of the generated moves
 */
void marscirce_generate_non_captures(PieNam p,
                                     square sq_generate_from,
                                     square sq_real_departure);

/* Generate capturing moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 * @param sq_real_departure real departure square of the generated moves
 */
void marscirce_generate_captures(PieNam p,
                                 square sq_generate_from,
                                 square sq_real_departure);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param p indicates the walk according to which to generate moves
 * @param sq_departure departure square of moves to be generated
 * @note the piece on the departure square need not have that walk
 */
void marscirce_generate_moves(PieNam p, square sq_departure);

/* Determine whether a specific piece delivers check to a specific side from a
 * specific rebirth square
 * @param pos_checking potentially delivering check ...
 * @param sq_rebrirth ... from this square
 * @note the piece on pos_checking must belong to advers(side)
 */
boolean mars_is_square_observed_by(square pos_checking,
                                   square sq_rebirth,
                                   square sq_target,
                                   evalfunction_t *evaluate);

/* Determine whether a side observes a specific square
 * @param side_observing the side
 * @param sq_target square potentially observed
 * @return true iff side is in check
 */
boolean marscirce_is_square_observed(square sq_target, evalfunction_t *evaluate);

#endif
