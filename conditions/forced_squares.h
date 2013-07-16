#if !defined(CONDITIONS_FORCED_SQUARES_H)
#define CONDITIONS_FORCED_SQUARES_H

/* This module implements forced squares */

#include "position/board.h"

/* Determine the length of a move in the presence of forced squares; the higher
 * the value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int forced_squares_measure_length_white(square sq_departure,
                                        square sq_arrival,
                                        square sq_capture);

/* Determine the length of a move in the presence of forced squares; the higher
 * the value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int forced_squares_measure_length_black(square sq_departure,
                                        square sq_arrival,
                                        square sq_capture);

#endif
