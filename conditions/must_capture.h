#if !defined(CONDITIONS_MUST_CAPTURE_H)
#define CONDITIONS_MUST_CAPTURE_H

/* This module implements the fairy conditions Black must capture and White
 * must capture */

#include "position/board.h"

/* Determine the length of a move in Black/White must capture; the higher
 * the value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int must_capture_measure_length(square sq_departure,
                                square sq_arrival,
                                square sq_capture);

#endif
