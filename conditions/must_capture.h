#if !defined(CONDITIONS_MUST_CAPTURE_H)
#define CONDITIONS_MUST_CAPTURE_H

/* This module implements the fairy conditions Black must capture and White
 * must capture */

#include "position/board.h"
#include "conditions/mummer.h"

/* Determine the length of a move in Black/White must capture; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type must_capture_measure_length(void);

#endif
