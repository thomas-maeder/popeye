#if !defined(CONDITIONS_FORCED_SQUARES_H)
#define CONDITIONS_FORCED_SQUARES_H

/* This module implements forced squares */

#include "position/board.h"
#include "conditions/mummer.h"

/* Determine the length of a move in the presence of forced squares; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type forced_squares_measure_length(void);

#endif
