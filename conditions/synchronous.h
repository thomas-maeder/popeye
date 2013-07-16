#if !defined(CONDITIONS_SYNCHRONOUS_H)
#define CONDITIONS_SYNCHRONOUS_H

/* This module implements Synchronous and Anti-synchronous Chess */

#include "position/board.h"

/* Determine the length of a move in Synchronous Chess; the higher
 * the value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int synchronous_measure_length(square sq_departure,
                               square sq_arrival,
                               square sq_capture);

/* Determine the length of a move in Anti-synchronous Chess; the higher
 * the value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int antisynchronous_measure_length(square sq_departure,
                                   square sq_arrival,
                                   square sq_capture);

#endif
