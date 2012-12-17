#if !defined(CONDITIONS_MARSCIRCE_MARSCIRCE_H)
#define CONDITIONS_MARSCIRCE_MARSCIRCE_H

#include "solving/solve.h"

/* This module provides implements the condition Mars-Circe
 */

/* Generate non-capturing moves
 * @param side side for which to generate moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 * @param sq_real_departure real departure square of the generated moves
 */
void marscirce_generate_non_captures(Side side,
                                     piece p,
                                     square sq_generate_from,
                                     square sq_real_departure);

/* Generate capturing moves
 * @param side side for which to generate moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 * @param sq_real_departure real departure square of the generated moves
 */
void marscirce_generate_captures(Side side,
                                 piece p,
                                 square sq_generate_from,
                                 square sq_real_departure);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param side side for which to generate moves for
 * @param p indicates the walk according to which to generate moves
 * @param sq_departure departure square of moves to be generated
 * @note the piece on the departure square need not have that walk
 */
void marscirce_generate_moves(Side side, piece p, square sq_departure);

#endif
