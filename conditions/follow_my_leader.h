#if !defined(CONDITIONS_FOLLOW_MY_LEADER_H)
#define CONDITIONS_FOLLOW_MY_LEADER_H

/* This module implements the fairy condition Follow my leader */

#include "position/board.h"

/* Determine the length of a move for the Follow my leader condition; the higher
 * the value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int follow_my_leader_measure_length(square sq_departure,
                                    square sq_arrival,
                                    square sq_capture);

#endif
