#if !defined(CONDITIONS_ANDERNACH_H)
#define CONDITIONS_ANDERNACH_H

#include "solving/solve.h"

/* This module implements the condition Andernach Chess */

/* Let the arriving piece of a move assume a side
 * @param side side to assume
 */
void andernach_assume_side(Side side);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type andernach_side_changer_solve(slice_index si,
                                               stip_length_type n);

/* Instrument slices with move tracers
 */
void stip_insert_andernach(slice_index si);

#endif
