#if !defined(CONDITIONS_FOLLOW_MY_LEADER_H)
#define CONDITIONS_FOLLOW_MY_LEADER_H

/* This module implements the fairy condition Follow my leader */

#include "position/board.h"
#include "conditions/mummer.h"

/* Determine the length of a move for the Follow my leader condition; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type follow_my_leader_measure_length(void);

#endif
