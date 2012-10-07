#if !defined(PIECES_ATTRIBUTES_HURDLE_COLOUR_CHANGING_H)
#define PIECES_ATTRIBUTES_HURDLE_COLOUR_CHANGING_H

/* This module implements the piece attribute "hurdle colour changing" */

#include "solving/solve.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type hurdle_colour_changer_solve(slice_index si,
                                                    stip_length_type n);

/* Instrument a stipulation for strict SAT
 * @param si identifies root slice of stipulation
 */
void stip_insert_hurdle_colour_changers(slice_index si);

#endif
