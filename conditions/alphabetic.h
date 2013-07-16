#if !defined(CONDITIONS_ALPHABETIC_H)
#define CONDITIONS_ALPHABETIC_H

/* This module implements the fairy condition Alphabetic Chess */

#include "position/board.h"

/* Determine the length of a move for Alphabetic Chess; the higher
 * the value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int alphabetic_measure_length(square sq_departure,
                              square sq_arrival,
                              square sq_capture);

#endif
