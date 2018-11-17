#if !defined(CONDITIONS_SYNCHRONOUS_H)
#define CONDITIONS_SYNCHRONOUS_H

#include "conditions/mummer.h"

/* This module implements Synchronous and Anti-synchronous Chess */

/* Determine the length of a move in Synchronous Chess; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type synchronous_measure_length(void);

/* Determine the length of a move in Anti-synchronous Chess; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type antisynchronous_measure_length(void);

#endif
