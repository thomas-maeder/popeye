#if !defined(CONDITIONS_DISTER_H)
#define CONDITIONS_DISTER_H

/* This module implements "Dister" conditions */

#include "solving/machinery/solve.h"
#include "conditions/mummer.h"

extern square dister_reference_square[2];

/* Determine the length of a move for the MaxDister conditions; the higher the
 * value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
mummer_length_type maxdister_measure_length(void);

/* Determine the length of a move for the MinDister conditions; the higher the
 * value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
mummer_length_type mindister_measure_length(void);

/* Initialise dister solving
 * @param si identifies root slice of stipulation
 */
void dister_initialise_solving(slice_index si);

#endif
